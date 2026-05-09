/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:27 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 22:22:04 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	launch_threads(t_context *context)
{
	int	i;

	i = 0;
	while (i < context->args.number_of_coders)
	{
		if (pthread_create(&context->coders[i].thread, NULL,
				coder_routine, &context->coders[i]) != 0)
			return (i);
		i++;
	}
	return (-1);
}

static void	join_threads(t_context *context, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(context->coders[i].thread, NULL);
		i++;
	}
}

static int	start_monitor(t_context *context)
{
	if (pthread_create(&context->monitor_thread, NULL,
			monitor_routine, context) != 0)
		return (0);
	return (1);
}

static int	stop_and_cleanup(t_context *context, int created)
{
	context_set_over(context);
	join_threads(context, created);
	destroy_context(context);
	return (1);
}

int	main(int argc, char **argv)
{
	t_args		args;
	t_context	context;
	int			created;

	if (!handle_args(argc, argv, &args))
		return (1);
	if (!init_context(&context, &args))
		return (1);
	created = launch_threads(&context);
	if (created != -1)
		return (stop_and_cleanup(&context, created));
	if (!start_monitor(&context))
		return (stop_and_cleanup(&context, context.args.number_of_coders));
	pthread_join(context.monitor_thread, NULL);
	join_threads(&context, context.args.number_of_coders);
	destroy_context(&context);
	return (0);
}
