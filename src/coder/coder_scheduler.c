/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_scheduler.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 14:11:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/13 18:06:10 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	coder_deadline_ms(t_coder *coder)
{
	t_context		*ctx;
	struct timeval	last;
	long			deadline;

	ctx = coder->context;
	pthread_mutex_lock(&ctx->state_mutex);
	last = coder->last_compile_time;
	pthread_mutex_unlock(&ctx->state_mutex);
	deadline = tv_to_ms(last) + ctx->args.time_to_burnout;
	return (deadline);
}

int	coder_scheduler_request_compile_slot(t_coder *coder)
{
	t_context	*ctx;
	long		deadline;

	ctx = coder->context;
	if (coder_should_stop(coder))
		return (0);
	deadline = coder_deadline_ms(coder);
	scheduler_request_slot(ctx, coder->id, deadline);
	return (1);
}

int	coder_scheduler_wait_for_turn(t_coder *coder)
{
	t_context	*ctx;

	ctx = coder->context;
	if (!scheduler_wait_turn(ctx, coder->id))
		return (0);
	return (!coder_should_stop(coder));
}

int	coder_scheduler_enter_compile_slot(t_coder *coder)
{
	if (!coder_scheduler_request_compile_slot(coder))
		return (0);
	return (coder_scheduler_wait_for_turn(coder));
}
