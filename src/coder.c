#include "codexion.h"

static void	set_lock_order(t_coder *coder, int *first, int *second)
{
	*first = coder->left_dongle;
	*second = coder->right_dongle;
	if (*first > *second)
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

static void	lock_dongles(t_coder *coder, int first, int second)
{
	t_sim		*sim;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	sim = coder->sim;
	first_dongle = &sim->dongles[first];
	second_dongle = &sim->dongles[second];
	pthread_mutex_lock(&first_dongle->mutex);
	printf("Coder %d has taken dongle %d.\n", coder->id, first_dongle->id);
	if (first != second)
	{
		pthread_mutex_lock(&second_dongle->mutex);
		printf("Coder %d has taken dongle %d.\n", coder->id,
			second_dongle->id);
	}
}

static void	unlock_dongles(t_coder *coder, int first, int second)
{
	t_sim		*sim;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	sim = coder->sim;
	first_dongle = &sim->dongles[first];
	second_dongle = &sim->dongles[second];
	if (first != second)
	{
		pthread_mutex_unlock(&second_dongle->mutex);
		printf("Coder %d has released dongle %d.\n", coder->id,
			second_dongle->id);
	}
	pthread_mutex_unlock(&first_dongle->mutex);
	printf("Coder %d has released dongle %d.\n", coder->id,
		first_dongle->id);
}

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_sim		*sim;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	sim = coder->sim;
	set_lock_order(coder, &first, &second);
	lock_dongles(coder, first, second);
	printf("Coder %d is compiling with dongles %d and %d.\n", coder->id,
		sim->dongles[coder->left_dongle].id,
		sim->dongles[coder->right_dongle].id);
	usleep(sim->args.time_to_compile);
	printf("Coder %d has finished compiling.\n", coder->id);
	unlock_dongles(coder, first, second);
	return (NULL);
}
