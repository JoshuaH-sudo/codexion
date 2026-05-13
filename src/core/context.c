/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:30 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/13 18:06:10 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_context *context, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&context->dongles[i].mutex);
		pthread_cond_destroy(&context->dongles[i].cooldown_cond);
		scheduler_destroy(&context->dongles[i].request_queue);
		i++;
	}
}

static int	init_dongles(t_context *context)
{
	int	i;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		context->dongles[i].id = i + 1;
		context->dongles[i].last_used_time.tv_sec = 0;
		context->dongles[i].last_used_time.tv_usec = 0;
		context->dongles[i].policy = (strcmp(context->args.scheduler, "fifo") == 0)
			? POLICY_FIFO : POLICY_EDF;
		if (pthread_cond_init(&context->dongles[i].cooldown_cond, NULL) != 0)
		{
			destroy_dongles(context, i);
			return (0);
		}
		if (pthread_mutex_init(&context->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongles(context, i);
			return (0);
		}
		if (!scheduler_init(&context->dongles[i].request_queue, context->args.number_of_coders,
				context->dongles[i].policy))
		{
			destroy_dongles(context, i);
			return (0);
		}
		i++;
	}
	return (1);
}

static int	init_state(t_context *context)
{
	context->monitor_thread = 0;
	context->simulation_over = 0;
	context->next_seq_no = 0;
	if (pthread_mutex_init(&context->state_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&context->scheduler_mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&context->scheduler_cond, NULL) != 0)
		return (0);
	if (!init_dongles(context) || !init_log_mutex(context))
	{
		pthread_mutex_destroy(&context->state_mutex);
		pthread_mutex_destroy(&context->scheduler_mutex);
		pthread_cond_destroy(&context->scheduler_cond);
		return (0);
	}
	return (1);
}

int	init_context(t_context *context, t_args *args)
{
	context->args = *args;
	context->coders = malloc(sizeof(t_coder) * context->args.number_of_coders);
	context->dongles = malloc(sizeof(t_dongle)
			* context->args.number_of_coders);
	if (!context->coders || !context->dongles)
	{
		free(context->coders);
		free(context->dongles);
		return (0);
	}
	if (!init_state(context))
	{
		free(context->coders);
		free(context->dongles);
		return (0);
	}
	if (!context_init_scheduler_heap(context))
	{
		destroy_context(context);
		return (0);
	}
	gettimeofday(&context->start_time, NULL);
	context_init_coders(context);
	return (1);
}

void	destroy_context(t_context *context)
{
	pthread_mutex_destroy(&context->log_mutex);
	pthread_mutex_destroy(&context->state_mutex);
	pthread_mutex_destroy(&context->scheduler_mutex);
	pthread_cond_destroy(&context->scheduler_cond);
	destroy_dongles(context, context->args.number_of_coders);
	scheduler_destroy(&context->scheduler_heap);
	free(context->coders);
	free(context->dongles);
}
