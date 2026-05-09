/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:27 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 15:55:27 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	launch_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (i);
		i++;
	}
	return (-1);
}

static void	join_threads(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_args	args;
	t_sim	sim;
	int		created;

	if (!handle_args(argc, argv, &args))
		return (1);
	if (!init_sim(&sim, &args))
		return (1);
	created = launch_threads(&sim);
	if (created != -1)
	{
		join_threads(&sim, created);
		destroy_sim(&sim);
		return (1);
	}
	join_threads(&sim, sim.args.number_of_coders);
	destroy_sim(&sim);
	return (0);
}
