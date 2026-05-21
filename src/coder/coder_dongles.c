/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_dongles.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:45:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 17:05:27 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	dongle_remaining_ms(t_dongle *dongle, int cooldown_ms)
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

static int	try_lock_ready(t_dongle *dongle, int cooldown_ms, long *wait_ms)
{
	long	remaining_ms;

	if (pthread_mutex_trylock(&dongle->mutex) != 0)
		return (0);
	remaining_ms = dongle_remaining_ms(dongle, cooldown_ms);
	if (remaining_ms > 0)
	{
		pthread_mutex_unlock(&dongle->mutex);
		*wait_ms = remaining_ms;
		return (0);
	}
	return (1);
}

static int	try_lock_pair(t_coder *coder, t_dongle *first_dongle,
		t_dongle *second_dongle, long *wait_ms)
{
	if (!try_lock_ready(first_dongle, coder->context->args.dongle_cooldown,
			wait_ms))
		return (0);
	if (first_dongle == second_dongle)
		return (log_message(coder, "has taken a dongle."), -1);
	if (!try_lock_ready(second_dongle, coder->context->args.dongle_cooldown,
			wait_ms))
	{
		pthread_mutex_unlock(&first_dongle->mutex);
		return (0);
	}
	log_message(coder, "has taken a dongle.");
	log_message(coder, "has taken a dongle.");
	return (1);
}

int	coder_lock_dongles(t_coder *coder, int first, int second)
{
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;
	long		wait_ms;
	int			status;

	first_dongle = &coder->context->dongles[first];
	second_dongle = &coder->context->dongles[second];
	while (!coder_should_stop(coder))
	{
		wait_ms = 1;
		status = try_lock_pair(coder, first_dongle, second_dongle, &wait_ms);
		if (status == 1)
			return (1);
		if (status == -1)
		{
			while (!coder_should_stop(coder))
				usleep(1000);
			pthread_mutex_unlock(&first_dongle->mutex);
			return (0);
		}
		usleep(wait_ms * 1000);
	}
	return (0);
}

void	coder_unlock_dongles(t_coder *coder, int first, int second)
{
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	first_dongle = &coder->context->dongles[first];
	second_dongle = &coder->context->dongles[second];
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
