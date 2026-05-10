/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 09:58:45 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/10 14:35:25 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_jobs(t_job *a, t_job *b)
{
	t_job	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

static int	job_less(const t_job *a, const t_job *b, t_policy policy)
{
	if (policy == POLICY_FIFO)
	{
		if (a->seq_no != b->seq_no)
			return (a->seq_no < b->seq_no);
		return (a->coder_id < b->coder_id);
	}
	if (a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->seq_no != b->seq_no)
		return (a->seq_no < b->seq_no);
	return (a->coder_id < b->coder_id);
}

static void	heapify_up(t_heap *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!job_less(&heap->data[idx], &heap->data[parent], heap->policy))
			break ;
		swap_jobs(&heap->data[idx], &heap->data[parent]);
		idx = parent;
	}
}

static void	heapify_down(t_heap *heap, int idx)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = idx * 2 + 1;
		right = idx * 2 + 2;
		smallest = idx;
		if (left < heap->size
			&& job_less(&heap->data[left], &heap->data[smallest], heap->policy))
			smallest = left;
		if (right < heap->size
			&& job_less(&heap->data[right], &heap->data[smallest], heap->policy))
			smallest = right;
		if (smallest == idx)
			break ;
		swap_jobs(&heap->data[idx], &heap->data[smallest]);
		idx = smallest;
	}
}

int	scheduler_init(t_heap *heap, int capacity, t_policy policy)
{
	heap->data = malloc(sizeof(t_job) * capacity);
	if (!heap->data)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	heap->policy = policy;
	return (1);
}

void	scheduler_destroy(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	scheduler_push_job(t_heap *heap, t_job job)
{
	if (heap->size >= heap->capacity)
		return (0);
	heap->data[heap->size] = job;
	heapify_up(heap, heap->size);
	heap->size++;
	return (1);
}

int	scheduler_pop_job(t_heap *heap, t_job *out)
{
	if (heap->size == 0)
		return (0);
	*out = heap->data[0];
	heap->size--;
	if (heap->size > 0)
	{
		heap->data[0] = heap->data[heap->size];
		heapify_down(heap, 0);
	}
	return (1);
}

int	scheduler_peek_job(t_heap *heap, t_job *out)
{
	if (!heap->size)
		return (0);
	*out = heap->data[0];
	return (1);
}

int	scheduler_is_empty(t_heap *heap)
{
	return (heap->size == 0);
}

void	scheduler_request_slot(t_context *context, int coder_id,
	long deadline_ms)
{
	t_job	job;

	job.coder_id = coder_id;
	job.seq_no = context->next_seq_no++;
	job.deadline_ms = deadline_ms;
	pthread_mutex_lock(&context->scheduler_mutex);
	scheduler_push_job(&context->scheduler_heap, job);
	pthread_cond_broadcast(&context->scheduler_cond);
	pthread_mutex_unlock(&context->scheduler_mutex);
}

int	scheduler_wait_turn(t_context *context, int coder_id)
{
	t_job	top;

	pthread_mutex_lock(&context->scheduler_mutex);
	while (!context->simulation_over
		&& !scheduler_is_empty(&context->scheduler_heap))
	{
		if (scheduler_peek_job(&context->scheduler_heap, &top)
			&& top.coder_id == coder_id)
		{
			scheduler_pop_job(&context->scheduler_heap, &top);
			pthread_mutex_unlock(&context->scheduler_mutex);
			return (1);
		}
		pthread_cond_wait(&context->scheduler_cond,
			&context->scheduler_mutex);
	}
	pthread_mutex_unlock(&context->scheduler_mutex);
	return (0);
}
