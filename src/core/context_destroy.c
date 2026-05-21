/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context_destroy.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 18:03:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 18:02:03 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	destroy_context(t_context *context)
{
	int	i;

	pthread_mutex_destroy(&context->log_mutex);
	pthread_mutex_destroy(&context->state_mutex);
	i = 0;
	while (i < context->args.number_of_coders)
	{
		pthread_mutex_destroy(&context->dongles[i].sched_mutex);
		pthread_cond_destroy(&context->dongles[i].sched_cond);
		scheduler_destroy(&context->dongles[i].queue);
		i++;
	}
	free(context->coders);
	free(context->dongles);
}
