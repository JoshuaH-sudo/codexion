/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:03:49 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 17:12:58 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <codexion.h>
#include <sys/time.h>

void log_message(t_coder	*coder, const char *message)
{
	t_context *context;
	struct timeval	tv;

	context = coder->context;
	gettimeofday(&tv, NULL);

	pthread_mutex_lock(&context->log_mutex);
	printf("%ld %d %s\n", tv.tv_sec, coder->id, message);
	pthread_mutex_unlock(&context->log_mutex);
}
