#include "codexion.h"

static void	destroy_dongles(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->dongles[i].id = i + 1;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongles(sim, i);
			return (0);
		}
		i++;
	}
	return (1);
}

static void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left_dongle = i;
		sim->coders[i].right_dongle =
			(i + 1) % sim->args.number_of_coders;
		sim->coders[i].sim = sim;
		i++;
	}
}

int	init_sim(t_sim *sim, t_args *args)
{
	sim->args = *args;
	sim->coders = malloc(sizeof(t_coder) *
			sim->args.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) *
			sim->args.number_of_coders);
	if (!sim->coders || !sim->dongles)
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	if (!init_dongles(sim))
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	init_coders(sim);
	return (1);
}

void	destroy_sim(t_sim *sim)
{
	destroy_dongles(sim, sim->args.number_of_coders);
	free(sim->coders);
	free(sim->dongles);
}
