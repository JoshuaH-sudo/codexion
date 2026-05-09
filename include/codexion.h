/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:34 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/09 17:11:43 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <pthread.h>

typedef struct s_args
{
	int	number_of_coders;
	int	time_to_burnout;
	int	time_to_compile;
	int	time_to_debug;
	int	time_to_refactor;
	int	number_of_compiles_required;
	int	dongle_cooldown;
	int	scheduler;
}		t_args;

typedef struct s_dongle
{
	int					id;
	pthread_mutex_t		mutex;
}			t_dongle;

typedef struct s_coder
{
	int					id;
	int					left_dongle;
	int					right_dongle;
	pthread_t			thread;
	struct s_context	*context;
}			t_coder;

typedef struct s_context
{
	t_args				args;
	t_coder				*coders;
	t_dongle			*dongles;
	pthread_mutex_t		log_mutex;
}			t_context;

int		handle_args(int argc, char **argv, t_args *args);
int		init_context(t_context *context, t_args *args);
void	destroy_context(t_context *context);
void	*coder_routine(void *arg);
void	log_message(t_coder *coder, const char *message);

#endif
