/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:34 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/18 17:26:00 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_policy
{
	POLICY_FIFO,
	POLICY_EDF
}						t_policy;

typedef struct s_job
{
	int					coder_id;
	long				seq_no;
	long				deadline_ms;
}						t_job;

typedef struct s_heap
{
	t_job				*data;
	int					size;
	int					capacity;
	t_policy			policy;
}						t_heap;

typedef struct s_args
{
	int					number_of_coders;
	int					time_to_burnout;
	int					time_to_compile;
	int					time_to_debug;
	int					time_to_refactor;
	int					number_of_compiles_required;
	int					dongle_cooldown;
	char				*scheduler;
}						t_args;

typedef struct s_dongle
{
	int					id;
	pthread_mutex_t		mutex;
	struct timeval		last_used_time;
	pthread_cond_t		cooldown_cond;
}						t_dongle;

typedef struct s_coder
{
	int					id;
	int					left_dongle;
	int					right_dongle;
	int					compiles_done;
	struct timeval		last_compile_time;
	pthread_t			thread;
	struct s_context	*context;
}						t_coder;

typedef struct s_context
{
	t_args				args;
	t_coder				*coders;
	t_dongle			*dongles;
	t_heap				scheduler_heap;
	pthread_t			monitor_thread;
	pthread_mutex_t		state_mutex;
	pthread_mutex_t		log_mutex;
	pthread_mutex_t		scheduler_mutex;
	pthread_cond_t		scheduler_cond;
	int					simulation_over;
	long				next_seq_no;
	struct timeval		start_time;
}						t_context;

int						handle_args(int argc, char **argv, t_args *args);
int						init_context(t_context *context, t_args *args);
void					destroy_context(t_context *context);
int						context_init_scheduler_heap(t_context *context);
void					*coder_routine(void *arg);
void					*monitor_routine(void *arg);
void					context_init_coders(t_context *context);
int						coder_scheduler_enter_compile_slot(t_coder *coder);
int						coder_lock_dongles(t_coder *coder, int first,
							int second);
void					coder_unlock_dongles(t_coder *coder, int first,
							int second);
long					coder_deadline_ms(t_coder *coder);
int						coder_should_stop(t_coder *coder);
void					mark_compile_start(t_coder *coder);
void					mark_compile_done(t_coder *coder);
void					context_set_over(t_context *context);
void					sleep_with_stop(t_coder *coder, int ms);
int						init_log_mutex(t_context *context);
void					log_message(t_coder *coder, const char *message);
int						scheduler_init(t_heap *heap, int capacity,
							t_policy policy);
void					scheduler_destroy(t_heap *heap);
int						scheduler_push_job(t_heap *heap, t_job job);
int						scheduler_pop_job(t_heap *heap, t_job *out);
int						scheduler_peek_job(t_heap *heap, t_job *out);
int						scheduler_is_empty(t_heap *heap);
void					scheduler_request_slot(t_context *context,
							int coder_id, long deadline_ms);
int						scheduler_wait_turn(t_context *context, int coder_id);
void					swap_heap_nodes(t_job *left_node,
							t_job *right_node);
int						job_has_higher_priority(const t_job *candidate,
							const t_job *current,
							const t_heap *queue);
void					sift_up_min_heap(t_heap *queue, int node_index);
void					sift_down_min_heap(t_heap *queue, int node_index);
long					tv_to_ms(struct timeval tv);
#endif
