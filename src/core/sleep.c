/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sleep.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/10 08:55:00 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/11 14:40:30 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sleep_with_stop(t_coder *coder, int ms)
{
	int	remaining;

	remaining = ms;
	while (remaining > 0 && !coder_should_stop(coder))
	{
		usleep(1000);
		remaining--;
	}
}
