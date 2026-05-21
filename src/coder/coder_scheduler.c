/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_scheduler.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 14:11:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 17:03:13 by jhoban           ###   ########.fr       */
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

int	coder_scheduler_enter_compile_slot(t_coder *coder)
{
	t_context	*ctx;

	ctx = coder->context;
	pthread_mutex_lock(&ctx->table_mutex);
	while (ctx->table_count >= ctx->table_max && !ctx->simulation_over)
		pthread_cond_wait(&ctx->table_cond, &ctx->table_mutex);
	if (ctx->simulation_over)
	{
		pthread_mutex_unlock(&ctx->table_mutex);
		return (0);
	}
	ctx->table_count++;
	pthread_mutex_unlock(&ctx->table_mutex);
	return (1);
}

void	coder_scheduler_leave_compile_slot(t_coder *coder)
{
	t_context	*ctx;

	ctx = coder->context;
	pthread_mutex_lock(&ctx->table_mutex);
	ctx->table_count--;
	pthread_cond_signal(&ctx->table_cond);
	pthread_mutex_unlock(&ctx->table_mutex);
}
