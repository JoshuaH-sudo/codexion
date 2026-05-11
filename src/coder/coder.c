/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:48:27 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_for_cooldown(t_coder *coder, t_dongle *dongle, int cooldown_ms)
{
	struct timeval	now;
	long			elapsed_ms;
	long			remaining;

	while (!coder_should_stop(coder))
	{
		gettimeofday(&now, NULL);
		elapsed_ms = (now.tv_sec - dongle->last_used_time.tv_sec) * 1000
			+ (now.tv_usec - dongle->last_used_time.tv_usec) / 1000;
		remaining = cooldown_ms - elapsed_ms;
		if (remaining <= 0)
			return (1);
		if (remaining > 1)
			usleep(1000);
		else
			usleep(remaining * 1000);
	}
	return (0);
}

static int	lock_dongles(t_coder *coder, int first, int second)
{
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
	if (!wait_for_cooldown(coder, first_dongle, context->args.dongle_cooldown))
		return (0);
	pthread_mutex_lock(&first_dongle->mutex);
	if (coder_should_stop(coder))
		return (pthread_mutex_unlock(&first_dongle->mutex), 0);
	log_message(coder, "has taken a dongle.");
	if (first == second)
		return (1);
	if (!wait_for_cooldown(coder, second_dongle, context->args.dongle_cooldown))
		return (pthread_mutex_unlock(&first_dongle->mutex), 0);
	pthread_mutex_lock(&second_dongle->mutex);
	if (coder_should_stop(coder))
		return (pthread_mutex_unlock(&second_dongle->mutex),
			pthread_mutex_unlock(&first_dongle->mutex), 0);
	log_message(coder, "has taken a dongle.");
	return (1);
}

static void	unlock_dongles(t_coder *coder, int first, int second)
{
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
	if (first != second)
	{
		pthread_mutex_unlock(&second_dongle->mutex);
		gettimeofday(&second_dongle->last_used_time, NULL);
		log_message(coder, "has released a dongle.");
	}
	pthread_mutex_unlock(&first_dongle->mutex);
	gettimeofday(&first_dongle->last_used_time, NULL);
	log_message(coder, "has released a dongle.");
}

static int	run_cycle(t_coder *coder, int first, int second)
{
	mark_compile_start(coder);
	if (coder_should_stop(coder))
		return (unlock_dongles(coder, first, second), 0);
	log_message(coder, "is compiling with dongles.");
	sleep_with_stop(coder, coder->context->args.time_to_compile);
	if (coder_should_stop(coder))
		return (unlock_dongles(coder, first, second), 0);
	log_message(coder, "has finished compiling.");
	unlock_dongles(coder, first, second);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is debugging");
	sleep_with_stop(coder, coder->context->args.time_to_debug);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is refactoring");
	sleep_with_stop(coder, coder->context->args.time_to_refactor);
	if (coder_should_stop(coder))
		return (0);
	return (mark_compile_done(coder), 1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	while (!coder_should_stop(coder))
	{
		if (!coder_scheduler_enter_compile_slot(coder))
			break ;
		first = coder->left_dongle;
		second = coder->right_dongle;
		if (first > second)
		{
			first = coder->right_dongle;
			second = coder->left_dongle;
		}
		if (!lock_dongles(coder, first, second))
			break ;
		if (!run_cycle(coder, first, second))
			break ;
	}
	return (NULL);
}
