/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/18 16:16:19 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	dongle_remaining_cooldown_ms(t_dongle *dongle, int cooldown_ms)
{
	struct timeval	now;
	long			elapsed_ms;
	long			remaining_ms;

	gettimeofday(&now, NULL);
	elapsed_ms = (now.tv_sec - dongle->last_used_time.tv_sec) * 1000
		+ (now.tv_usec - dongle->last_used_time.tv_usec) / 1000;
	remaining_ms = cooldown_ms - elapsed_ms;
	if (remaining_ms < 0)
		return (0);
	return (remaining_ms);
}

static int	try_lock_ready_dongle(t_dongle *dongle, int cooldown_ms,
		long *wait_ms)
{
	long	remaining_ms;

	if (pthread_mutex_trylock(&dongle->mutex) != 0)
		return (0);
	remaining_ms = dongle_remaining_cooldown_ms(dongle, cooldown_ms);
	if (remaining_ms > 0)
	{
		pthread_mutex_unlock(&dongle->mutex);
		*wait_ms = remaining_ms;
		return (0);
	}
	return (1);
}

static int	lock_dongles(t_coder *coder, int first, int second)
{
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;
	long		wait_ms;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
	while (!coder_should_stop(coder))
	{
		wait_ms = 1;
		if (!try_lock_ready_dongle(first_dongle, context->args.dongle_cooldown,
				&wait_ms))
		{
			usleep(wait_ms * 1000);
			continue ;
		}
		if (first == second)
			return (log_message(coder, "has taken a dongle."), 1);
		if (!try_lock_ready_dongle(second_dongle,
				context->args.dongle_cooldown, &wait_ms))
		{
			pthread_mutex_unlock(&first_dongle->mutex);
			usleep(wait_ms * 1000);
			continue ;
		}
		log_message(coder, "has taken a dongle.");
		log_message(coder, "has taken a dongle.");
		return (1);
	}
	return (0);
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
		gettimeofday(&second_dongle->last_used_time, NULL);
		pthread_mutex_unlock(&second_dongle->mutex);
		log_message(coder, "has released a dongle.");
	}
	gettimeofday(&first_dongle->last_used_time, NULL);
	pthread_mutex_unlock(&first_dongle->mutex);
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
