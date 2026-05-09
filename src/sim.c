/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:30 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 16:13:13 by jhoban           ###   ########.fr       */
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
		if (pthread_mutex_init(&context->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongles(context, i);
			return (0);
		}
		i++;
	}
	return (1);
}

static void	init_coders(t_context *context)
{
	int	i;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		context->coders[i].id = i + 1;
		context->coders[i].left_dongle = i;
		context->coders[i].right_dongle
			= (i + 1) % context->args.number_of_coders;
		context->coders[i].context = context;
		i++;
	}
}

int	init_context(t_context *context, t_args *args)
{
	context->args = *args;
	context->coders = malloc(sizeof(t_coder)
			* context->args.number_of_coders);
	context->dongles = malloc(sizeof(t_dongle)
			* context->args.number_of_coders);
	if (!context->coders || !context->dongles)
	{
		free(context->coders);
		free(context->dongles);
		return (0);
	}
	if (!init_dongles(context))
	{
		free(context->coders);
		free(context->dongles);
		return (0);
	}
	init_coders(context);
	return (1);
}

void	destroy_context(t_context *context)
{
	destroy_dongles(context, context->args.number_of_coders);
	free(context->coders);
	free(context->dongles);
}
