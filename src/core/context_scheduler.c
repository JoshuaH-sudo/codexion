/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context_scheduler.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 13:07:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 17:03:35 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_policy	context_get_policy(t_context *context)
{
	if (strcmp(context->args.scheduler, "edf") == 0)
		return (POLICY_EDF);
	return (POLICY_FIFO);
}

int	context_compute_table_max(t_context *ctx)
{
	int	step;
	int	tmax;

	step = ctx->args.time_to_compile + ctx->args.dongle_cooldown;
	if (step <= 0)
		return (ctx->args.number_of_coders);
	tmax = ctx->args.time_to_burnout / step;
	if (tmax < 1)
		tmax = 1;
	if (tmax > ctx->args.number_of_coders)
		return (ctx->args.number_of_coders);
	return (tmax);
}
