/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 22:24:06 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_for_cooldown(t_dongle *dongle, int cooldown_ms)
{
	struct timeval	now;
	long			elapsed_ms;

	gettimeofday(&now, NULL);
	elapsed_ms = (now.tv_sec - dongle->last_used_time.tv_sec) * 1000
		+ (now.tv_usec - dongle->last_used_time.tv_usec) / 1000;
	if (elapsed_ms < cooldown_ms)
		usleep((cooldown_ms - elapsed_ms) * 1000);
}

static void	set_lock_order(t_coder *coder, int *first, int *second)
{
	*first = coder->left_dongle;
	*second = coder->right_dongle;
	if (*first > *second)
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

static void	lock_dongles(t_coder *coder, int first, int second)
{
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
	wait_for_cooldown(first_dongle, context->args.dongle_cooldown);
	pthread_mutex_lock(&first_dongle->mutex);
	log_message(coder, "has taken a dongle.");
	if (first != second)
	{
		wait_for_cooldown(second_dongle, context->args.dongle_cooldown);
		pthread_mutex_lock(&second_dongle->mutex);
		log_message(coder, "has taken a dongle.");
	}
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

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	int			first;
	int			second;

	coder = (t_coder *)arg;
	while (!coder_should_stop(coder))
	{
		set_lock_order(coder, &first, &second);
		lock_dongles(coder, first, second);
		mark_compile_start(coder);
		log_message(coder, "is compiling with dongles.");
		usleep(coder->context->args.time_to_compile * 1000);
		log_message(coder, "has finished compiling.");
		unlock_dongles(coder, first, second);
		if (coder_should_stop(coder))
			break ;
		log_message(coder, "is debugging");
		usleep(coder->context->args.time_to_debug * 1000);
		log_message(coder, "is refactoring");
		usleep(coder->context->args.time_to_refactor * 1000);
		mark_compile_done(coder);
	}
	return (NULL);
}
