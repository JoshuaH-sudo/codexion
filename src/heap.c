/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:40:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 12:50:15 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_swap_jobs(t_job *a, t_job *b)
{
	t_job	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

int	heap_job_less(const t_job *a, const t_job *b, t_policy policy)
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

void	heap_heapify_up(t_heap *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!heap_job_less(&heap->data[idx], &heap->data[parent],
				heap->policy))
			break ;
		heap_swap_jobs(&heap->data[idx], &heap->data[parent]);
		idx = parent;
	}
}

void	heap_heapify_down(t_heap *heap, int idx)
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
			&& heap_job_less(&heap->data[left], &heap->data[smallest],
				heap->policy))
			smallest = left;
		if (right < heap->size
			&& heap_job_less(&heap->data[right], &heap->data[smallest],
				heap->policy))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap_jobs(&heap->data[idx], &heap->data[smallest]);
		idx = smallest;
	}
}
