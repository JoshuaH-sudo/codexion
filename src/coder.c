/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 16:07:09 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
	pthread_mutex_lock(&first_dongle->mutex);
	printf("Coder %d has taken dongle %d.\n", coder->id,
		first_dongle->id);
	if (first != second)
	{
		pthread_mutex_lock(&second_dongle->mutex);
		printf("Coder %d has taken dongle %d.\n", coder->id,
			second_dongle->id);
	}
}

static void	unlock_dongles(t_coder *coder, int first, int second)
{
	t_context	*context;
	t_dongle	*first_dongle;
	t_dongle	*second_dongle;

	context = coder->context;
	first_dongle = &context->dongles[first];
	second_dongle = &context->dongles[second];
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
	t_context	*context;
	int			first;
	int			second;

	coder = (t_coder *)arg;
	context = coder->context;
	set_lock_order(coder, &first, &second);
	lock_dongles(coder, first, second);
	printf("Coder %d is compiling with dongles %d and %d.\n",
		coder->id,
		context->dongles[coder->left_dongle].id,
		context->dongles[coder->right_dongle].id);
	usleep(context->args.time_to_compile);
	printf("Coder %d has finished compiling.\n", coder->id);
	unlock_dongles(coder, first, second);
	return (NULL);
}
