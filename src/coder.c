/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 17:11:35 by jhoban           ###   ########.fr       */
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
	log_message(coder, "has taken a dongle.");
	if (first != second)
	{
		pthread_mutex_lock(&second_dongle->mutex);
		log_message(coder, "has taken a dongle.");
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
		log_message(coder, "has released a dongle.");
	}
	pthread_mutex_unlock(&first_dongle->mutex);
	log_message(coder, "has released a dongle.");
}

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_context	*context;
	int			first;
	int			second;
	int			compiles;

	coder = (t_coder *)arg;
	context = coder->context;
	compiles = 0;
	while (compiles < context->args.number_of_compiles_required)
	{
		set_lock_order(coder, &first, &second);
		lock_dongles(coder, first, second);
		log_message(coder, "is compiling with dongles.");
		usleep(context->args.time_to_compile);
		log_message(coder, "has finished compiling.");
		unlock_dongles(coder, first, second);
		compiles++;
	}
	return (NULL);
}
