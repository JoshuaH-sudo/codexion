/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 09:58:45 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:22:35 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
	sift_up_min_heap(heap, heap->size);
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
		sift_down_min_heap(heap, 0);
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
