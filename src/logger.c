/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:03:49 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/10 08:56:27 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <codexion.h>
#include <sys/time.h>

static int	can_log_message(t_context *context, const char *message)
{
	int	can_log;

	can_log = 1;
	pthread_mutex_lock(&context->state_mutex);
	if (context->simulation_over && strcmp(message, "burned out") != 0)
		can_log = 0;
	pthread_mutex_unlock(&context->state_mutex);
	return (can_log);
}

int	init_log_mutex(t_context *context)
{
	if (pthread_mutex_init(&context->log_mutex, NULL) != 0)
		return (0);
	return (1);
}

void	log_message(t_coder *coder, const char *message)
{
	t_context		*context;
	struct timeval	tv;
	long			elapsed_ms;

	context = coder->context;
	if (!can_log_message(context, message))
		return ;
	gettimeofday(&tv, NULL);
	elapsed_ms = (tv.tv_sec - context->start_time.tv_sec) * 1000
		+ (tv.tv_usec - context->start_time.tv_usec) / 1000;
	pthread_mutex_lock(&context->log_mutex);
	if (can_log_message(context, message))
		printf("%ld %d %s\n", elapsed_ms, coder->id, message);
	pthread_mutex_unlock(&context->log_mutex);
}
