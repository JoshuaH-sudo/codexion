/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_sync.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 13:07:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 17:03:35 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	scheduler_is_empty(t_heap *heap)
{
	return (heap->size == 0);
}

void	dongle_request_access(t_dongle *dongle, t_context *ctx,
	int coder_id, long deadline_ms)
{
	t_job	job;

	pthread_mutex_lock(&dongle->sched_mutex);
	job.coder_id = coder_id;
	job.seq_no = dongle->next_seq_no++;
	job.deadline_ms = deadline_ms;
	scheduler_push_job(&dongle->queue, job);
	pthread_cond_broadcast(&dongle->sched_cond);
	pthread_mutex_unlock(&dongle->sched_mutex);
	(void)ctx;
}

static long	cooldown_remaining(t_dongle *dongle, int cooldown_ms)
{
	struct timeval	now;
	long			elapsed_ms;

	gettimeofday(&now, NULL);
	elapsed_ms = (now.tv_sec - dongle->last_used_time.tv_sec) * 1000
		+ (now.tv_usec - dongle->last_used_time.tv_usec) / 1000;
	if (elapsed_ms >= cooldown_ms)
		return (0);
	return (cooldown_ms - elapsed_ms);
}

int	dongle_wait_access(t_dongle *dongle, t_context *ctx,
	int coder_id, int cooldown_ms)
{
	t_job			top;
	long			remaining;
	struct timespec	ts;
	struct timeval	now;

	pthread_mutex_lock(&dongle->sched_mutex);
	while (!ctx->simulation_over)
	{
		if (!scheduler_peek_job(&dongle->queue, &top)
			|| top.coder_id != coder_id || dongle->held)
		{
			pthread_cond_wait(&dongle->sched_cond, &dongle->sched_mutex);
			continue ;
		}
		remaining = cooldown_remaining(dongle, cooldown_ms);
		if (remaining <= 0)
		{
			scheduler_pop_job(&dongle->queue, &top);
			dongle->held = 1;
			pthread_mutex_unlock(&dongle->sched_mutex);
			return (1);
		}
		gettimeofday(&now, NULL);
		ts.tv_sec = now.tv_sec + remaining / 1000;
		ts.tv_nsec = (long)now.tv_usec * 1000 + (remaining % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000)
		{
			ts.tv_sec++;
			ts.tv_nsec -= 1000000000;
		}
		pthread_cond_timedwait(&dongle->sched_cond, &dongle->sched_mutex, &ts);
	}
	pthread_mutex_unlock(&dongle->sched_mutex);
	return (0);
}

void	dongle_release_access(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->sched_mutex);
	dongle->held = 0;
	gettimeofday(&dongle->last_used_time, NULL);
	pthread_cond_broadcast(&dongle->sched_cond);
	pthread_mutex_unlock(&dongle->sched_mutex);
}
