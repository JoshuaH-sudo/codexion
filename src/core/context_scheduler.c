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
