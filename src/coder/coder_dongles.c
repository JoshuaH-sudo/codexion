/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_dongles.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:45:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/21 17:16:18 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	lock_single_dongle(t_coder *coder, int idx)
{
	t_context	*ctx;
	long		deadline;

	ctx = coder->context;
	deadline = coder_deadline_ms(coder);
	dongle_request_access(&ctx->dongles[idx], ctx, coder->id, deadline);
	if (!dongle_wait_access(&ctx->dongles[idx], ctx, coder->id,
			ctx->args.dongle_cooldown))
		return (0);
	log_message(coder, "has taken a dongle.");
	return (1);
}

int	coder_lock_dongles(t_coder *coder, int first, int second)
{
	t_context	*ctx;

	ctx = coder->context;
	if (first == second)
	{
		while (!coder_should_stop(coder))
			usleep(1000);
		return (0);
	}
	if (!lock_single_dongle(coder, first))
		return (0);
	if (coder_should_stop(coder))
	{
		dongle_release_access(&ctx->dongles[first]);
		return (0);
	}
	if (!lock_single_dongle(coder, second))
	{
		dongle_release_access(&ctx->dongles[first]);
		return (0);
	}
	return (1);
}

void	coder_unlock_dongles(t_coder *coder, int first, int second)
{
	t_context	*ctx;

	ctx = coder->context;
	if (first != second)
	{
		dongle_release_access(&ctx->dongles[second]);
		log_message(coder, "has released a dongle.");
	}
	dongle_release_access(&ctx->dongles[first]);
	log_message(coder, "has released a dongle.");
}
