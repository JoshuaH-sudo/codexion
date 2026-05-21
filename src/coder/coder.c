/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:41 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/18 19:06:26 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	run_cycle(t_coder *coder, int first, int second)
{
	if (coder_should_stop(coder))
		return (coder_unlock_dongles(coder, first, second), 0);
	mark_compile_start(coder);
	log_message(coder, "is compiling with dongles.");
	sleep_with_stop(coder, coder->context->args.time_to_compile);
	if (coder_should_stop(coder))
		return (coder_unlock_dongles(coder, first, second), 0);
	log_message(coder, "has finished compiling.");
	coder_unlock_dongles(coder, first, second);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is debugging");
	sleep_with_stop(coder, coder->context->args.time_to_debug);
	if (coder_should_stop(coder))
		return (0);
	log_message(coder, "is refactoring");
	sleep_with_stop(coder, coder->context->args.time_to_refactor);
	if (coder_should_stop(coder))
		return (0);
	return (mark_compile_done(coder), 1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	while (!coder_should_stop(coder))
	{
		if (!coder_scheduler_enter_compile_slot(coder))
			break ;
		first = coder->left_dongle;
		second = coder->right_dongle;
		if (first > second)
		{
			first = coder->right_dongle;
			second = coder->left_dongle;
		}
		if (!coder_lock_dongles(coder, first, second))
			break ;
		if (!run_cycle(coder, first, second))
			break ;
	}
	return (NULL);
}
