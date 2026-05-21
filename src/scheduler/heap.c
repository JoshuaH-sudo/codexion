/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:40:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 18:02:03 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	scheduler_is_empty(t_heap *heap)
{
	return (heap->size == 0);
}

void	swap_heap_nodes(t_job *left_node, t_job *right_node)
{
	t_job	temp;

	temp = *left_node;
	*left_node = *right_node;
	*right_node = temp;
}

int	job_has_higher_priority(const t_job *candidate, const t_job *current,
		const t_heap *queue)
{
	if (queue->policy == POLICY_FIFO)
		return (candidate->seq_no < current->seq_no);
	if (candidate->deadline_ms != current->deadline_ms)
		return (candidate->deadline_ms < current->deadline_ms);
	return (candidate->seq_no < current->seq_no);
}

void	sift_up_min_heap(t_heap *queue, int node_index)
{
	int	parent;

	while (node_index > 0)
	{
		parent = (node_index - 1) / 2;
		if (!job_has_higher_priority(&queue->data[node_index],
				&queue->data[parent], queue))
			break ;
		swap_heap_nodes(&queue->data[node_index], &queue->data[parent]);
		node_index = parent;
	}
}

void	sift_down_min_heap(t_heap *queue, int node_index)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = node_index * 2 + 1;
		right = node_index * 2 + 2;
		smallest = node_index;
		if (left < queue->size && job_has_higher_priority(&queue->data[left],
				&queue->data[smallest], queue))
			smallest = left;
		if (right < queue->size && job_has_higher_priority(&queue->data[right],
				&queue->data[smallest], queue))
			smallest = right;
		if (smallest == node_index)
			break ;
		swap_heap_nodes(&queue->data[node_index], &queue->data[smallest]);
		node_index = smallest;
	}
}
