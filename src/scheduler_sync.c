/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_sync.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 13:07:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:10:55 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	scheduler_is_empty(t_heap *heap)
{
	return (heap->size == 0);
}

void	scheduler_request_slot(t_context *context, int coder_id,
	long deadline_ms)
{
	t_job	job;

	pthread_mutex_lock(&context->scheduler_mutex);
	job.coder_id = coder_id;
	job.seq_no = context->next_seq_no++;
	job.deadline_ms = deadline_ms;
	scheduler_push_job(&context->scheduler_heap, job);
	pthread_cond_broadcast(&context->scheduler_cond);
	pthread_mutex_unlock(&context->scheduler_mutex);
}

int	scheduler_wait_turn(t_context *context, int coder_id)
{
	t_job	top;

	pthread_mutex_lock(&context->scheduler_mutex);
	while (!context->simulation_over)
	{
		if (scheduler_peek_job(&context->scheduler_heap, &top)
			&& top.coder_id == coder_id)
		{
			scheduler_pop_job(&context->scheduler_heap, &top);
			pthread_cond_broadcast(&context->scheduler_cond);
			pthread_mutex_unlock(&context->scheduler_mutex);
			return (1);
		}
		pthread_cond_wait(&context->scheduler_cond,
			&context->scheduler_mutex);
	}
	pthread_mutex_unlock(&context->scheduler_mutex);
	return (0);
}
