#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_sim		*sim;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	sim = coder->sim;
	left_dongle = &sim->dongles[coder->left_dongle];
	right_dongle = &sim->dongles[coder->right_dongle];
	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first > second)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
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
	printf("Coder %d is compiling with dongles %d and %d.\n", coder->id,
		left_dongle->id, right_dongle->id);
	usleep(sim->args.time_to_compile);
	printf("Coder %d has finished compiling.\n", coder->id);
	if (first != second)
	{
		pthread_mutex_unlock(&second_dongle->mutex);
		printf("Coder %d has released dongle %d.\n", coder->id,
			second_dongle->id);
	}
	pthread_mutex_unlock(&first_dongle->mutex);
	printf("Coder %d has released dongle %d.\n", coder->id,
		first_dongle->id);
	(void)coder;
	return (NULL);
}
