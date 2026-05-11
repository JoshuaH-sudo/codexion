/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context_scheduler.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 13:07:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:40:30 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	context_init_scheduler_heap(t_context *context)
{
	if (strcmp(context->args.scheduler, "fifo") == 0)
	{
		if (!scheduler_init(&context->scheduler_heap,
				context->args.number_of_coders, POLICY_FIFO))
			return (0);
	}
	else
	{
		if (!scheduler_init(&context->scheduler_heap,
				context->args.number_of_coders, POLICY_EDF))
			return (0);
	}
	return (1);
}
