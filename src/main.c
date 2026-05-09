/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:27 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 16:05:38 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	launch_threads(t_sim *context)
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

static void	join_threads(t_sim *context, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(context->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_args	args;
	t_sim	context;
	int		created;

	if (!handle_args(argc, argv, &args))
		return (1);
	if (!init_sim(&context, &args))
		return (1);
	created = launch_threads(&context);
	if (created != -1)
	{
		join_threads(&context, created);
		destroy_sim(&context);
		return (1);
	}
	join_threads(&context, context.args.number_of_coders);
	destroy_sim(&context);
	return (0);
}
