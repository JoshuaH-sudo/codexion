/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/13 18:15:46 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <time.h>

static void	remove_request_from_queue(t_heap *queue, int coder_id)
{
	int	index;

	index = 0;
	while (index < queue->size)
	{
		if (queue->data[index].coder_id == coder_id)
		{
			queue->size--;
			if (index != queue->size)
			{
				queue->data[index] = queue->data[queue->size];
				sift_down_min_heap(queue, index);
				sift_up_min_heap(queue, index);
			}
			return ;
		}
		index++;
	}
}

static int	lock_dongle(t_coder *coder, t_dongle *dongle, int cooldown_ms)
{
	t_context	*ctx;
	t_job		request;
	t_job		top;
	long		elapsed;
	struct timeval	now;

	ctx = coder->context;
	pthread_mutex_lock(&dongle->mutex);

	request.coder_id = coder->id;
	request.seq_no = __sync_fetch_and_add(&ctx->next_seq_no, 1);
	request.deadline_ms = coder_deadline_ms(coder);
	if (!scheduler_push_job(&dongle->request_queue, request))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}

	while (!coder_should_stop(coder))
	{
		if (scheduler_peek_job(&dongle->request_queue, &top) && top.coder_id == coder->id)
		{
			gettimeofday(&now, NULL);
			elapsed = (now.tv_sec - dongle->last_used_time.tv_sec) * 1000
				+ (now.tv_usec - dongle->last_used_time.tv_usec) / 1000;
			if (elapsed >= cooldown_ms)
			{
				scheduler_pop_job(&dongle->request_queue, &top);
				pthread_mutex_unlock(&dongle->mutex);
				log_message(coder, "has taken a dongle.");
				return (1);
			}
		}
		pthread_cond_timedwait(&dongle->cooldown_cond, &dongle->mutex,
			&(struct timespec){.tv_sec = time(NULL) + 1, .tv_nsec = 0});
	}
	remove_request_from_queue(&dongle->request_queue, coder->id);
	pthread_cond_broadcast(&dongle->cooldown_cond);
	pthread_mutex_unlock(&dongle->mutex);
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

	if (!lock_dongle(coder, first_dongle, context->args.dongle_cooldown))
		return (0);

	if (first == second)
		return (1);

	if (!lock_dongle(coder, second_dongle, context->args.dongle_cooldown))
	{
		pthread_mutex_lock(&first_dongle->mutex);
		gettimeofday(&first_dongle->last_used_time, NULL);
		pthread_cond_broadcast(&first_dongle->cooldown_cond);
		pthread_mutex_unlock(&first_dongle->mutex);
		log_message(coder, "has released a dongle.");
		return (0);
	}
	return (1);
}

static void	unlock_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	gettimeofday(&dongle->last_used_time, NULL);
	pthread_cond_broadcast(&dongle->cooldown_cond);
	pthread_mutex_unlock(&dongle->mutex);
	log_message(coder, "has released a dongle.");
}

static int	run_cycle(t_coder *coder, int first, int second)
{
	t_context	*context;

	context = coder->context;
	mark_compile_start(coder);
	if (coder_should_stop(coder))
	{
		unlock_dongle(coder, &context->dongles[first]);
		if (first != second)
			unlock_dongle(coder, &context->dongles[second]);
		return (0);
	}
	log_message(coder, "is compiling with dongles.");
	sleep_with_stop(coder, context->args.time_to_compile);
	if (coder_should_stop(coder))
	{
		unlock_dongle(coder, &context->dongles[first]);
		if (first != second)
			unlock_dongle(coder, &context->dongles[second]);
		return (0);
	}
	log_message(coder, "has finished compiling.");
	unlock_dongle(coder, &context->dongles[first]);
	if (first != second)
		unlock_dongle(coder, &context->dongles[second]);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is debugging");
	sleep_with_stop(coder, context->args.time_to_debug);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is refactoring");
	sleep_with_stop(coder, context->args.time_to_refactor);
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
