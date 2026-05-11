/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 22:30:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:40:30 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	context_init_coders(t_context *context)
{
	int	i;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		context->coders[i].id = i + 1;
		context->coders[i].left_dongle = i;
		context->coders[i].right_dongle
			= (i + 1) % context->args.number_of_coders;
		context->coders[i].compiles_done = 0;
		context->coders[i].last_compile_time = context->start_time;
		context->coders[i].context = context;
		i++;
	}
}

int	coder_should_stop(t_coder *coder)
{
	t_context	*context;
	int			stop;

	context = coder->context;
	pthread_mutex_lock(&context->state_mutex);
	stop = (context->simulation_over
			|| coder->compiles_done
			>= context->args.number_of_compiles_required);
	pthread_mutex_unlock(&context->state_mutex);
	return (stop);
}

void	mark_compile_start(t_coder *coder)
{
	t_context	*context;

	context = coder->context;
	pthread_mutex_lock(&context->state_mutex);
	gettimeofday(&coder->last_compile_time, NULL);
	pthread_mutex_unlock(&context->state_mutex);
}

void	mark_compile_done(t_coder *coder)
{
	t_context	*context;

	context = coder->context;
	pthread_mutex_lock(&context->state_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&context->state_mutex);
}

void	context_set_over(t_context *context)
{
	pthread_mutex_lock(&context->state_mutex);
	context->simulation_over = 1;
	pthread_mutex_unlock(&context->state_mutex);
	pthread_mutex_lock(&context->scheduler_mutex);
	pthread_cond_broadcast(&context->scheduler_cond);
	pthread_mutex_unlock(&context->scheduler_mutex);
}
