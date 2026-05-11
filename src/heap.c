/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 14:40:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:22:35 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	swap_heap_nodes(t_job *left_node, t_job *right_node)
{
	t_job	temp;

	temp = *left_node;
	*left_node = *right_node;
	*right_node = temp;
}

int	job_has_higher_priority(const t_job *candidate,
	const t_job *current, t_policy scheduler_policy, int starvation_window)
{
	long	gap;

	if (scheduler_policy == POLICY_FIFO)
	{
		if (candidate->seq_no != current->seq_no)
			return (candidate->seq_no < current->seq_no);
		return (candidate->coder_id < current->coder_id);
	}
	gap = candidate->seq_no - current->seq_no;
	if (gap < 0)
		gap = -gap;
	if (gap >= starvation_window)
		return (candidate->seq_no < current->seq_no);
	if (candidate->deadline_ms != current->deadline_ms)
		return (candidate->deadline_ms < current->deadline_ms);
	if (candidate->seq_no != current->seq_no)
		return (candidate->seq_no < current->seq_no);
	return (candidate->coder_id < current->coder_id);
}

void	sift_up_min_heap(t_heap *queue, int node_index)
{
	int	parent;

	while (node_index > 0)
	{
		parent = (node_index - 1) / 2;
		if (!job_has_higher_priority(&queue->data[node_index],
				&queue->data[parent], queue->policy, queue->capacity))
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
		if (left < queue->size
			&& job_has_higher_priority(&queue->data[left],
				&queue->data[smallest], queue->policy, queue->capacity))
			smallest = left;
		if (right < queue->size
			&& job_has_higher_priority(&queue->data[right],
				&queue->data[smallest], queue->policy, queue->capacity))
			smallest = right;
		if (smallest == node_index)
			break ;
		swap_heap_nodes(&queue->data[node_index], &queue->data[smallest]);
		node_index = smallest;
	}
}
