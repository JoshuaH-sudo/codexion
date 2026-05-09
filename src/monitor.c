/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 22:22:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 22:20:03 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	elapsed_ms(struct timeval from, struct timeval to)
{
	return ((to.tv_sec - from.tv_sec) * 1000
		+ (to.tv_usec - from.tv_usec) / 1000);
}

static int	all_coders_done(t_context *context)
{
	int	i;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		if (context->coders[i].compiles_done
			< context->args.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	burned_coder_index(t_context *context, struct timeval now)
{
	int				i;
	long			elapsed;
	struct timeval	last;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		last = context->coders[i].last_compile_time;
		elapsed = elapsed_ms(last, now);
		if (context->coders[i].compiles_done
			< context->args.number_of_compiles_required
			&& elapsed >= context->args.time_to_burnout)
			return (i);
		i++;
	}
	return (-1);
}

void	*monitor_routine(void *arg)
{
	t_context		*context;
	struct timeval	now;
	int				burned;

	context = (t_context *)arg;
	while (1)
	{
		gettimeofday(&now, NULL);
		pthread_mutex_lock(&context->state_mutex);
		if (context->simulation_over || all_coders_done(context))
		{
			context->simulation_over = 1;
			pthread_mutex_unlock(&context->state_mutex);
			return (NULL);
		}
		burned = burned_coder_index(context, now);
		if (burned != -1)
		{
			context->simulation_over = 1;
			pthread_mutex_unlock(&context->state_mutex);
			log_message(&context->coders[burned], "burned out");
			return (NULL);
		}
		pthread_mutex_unlock(&context->state_mutex);
		usleep(1000);
	}
	return (NULL);
}
