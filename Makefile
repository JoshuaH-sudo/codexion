NAME		= codexion
STRESS_NAME	= stress_test

CC		= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I include

SRCS		= src/main.c \
			src/parsing/args.c \
			src/core/context.c \
			src/core/context_scheduler.c \
			src/coder/coder.c \
			src/coder/coder_dongles.c \
			src/coder/coder_scheduler.c \
			src/core/logger.c \
			src/core/monitor.c \
			src/core/state.c \
			src/core/sleep.c \
			src/scheduler/scheduler.c \
			src/scheduler/scheduler_sync.c \
			src/scheduler/heap.c \
			src/common/utils.c \

# Arguments:
# <number_of_coders>
# <time_to_burnout>
# <time_to_compile>
# <time_to_debug>
# <time_to_refactor>
# <number_of_compiles_required>
# <dongle_cooldown>
# <scheduler>
ARGS		?= 199 900 60 60 60 3 60 fifo
CONSISTENCY_ARGS	?= 10 500 80 80 80 3 10 edf
CONSISTENCY_RUNS	?= 10
BATCHES		?= 20
RUNS_PER_BATCH	?= 10
MATRIX_RUNS	?= 5

OBJS		= $(SRCS:src/%.c=obj/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

# run with short timeouts for testing:
# make run ARGS="5 100 50 50 50 3 100 fifo"
# Run with default arguments:
# make run ARGS="5 800 200 200 200 3 50 fifo"
# Run with edf
# make run ARGS="5 800 100 100 100 3 10 edf"
run: $(NAME)
	./$(NAME) $(ARGS)

smoke: $(NAME)
	./scripts/smoke_tests.sh

consistency: $(NAME)
	ARGS="$(CONSISTENCY_ARGS)" RUNS="$(CONSISTENCY_RUNS)" ./scripts/consistency_test.sh

consistency-batches: $(NAME)
	ARGS="$(CONSISTENCY_ARGS)" ./scripts/consistency_batches.sh $(BATCHES) $(RUNS_PER_BATCH)

edge-matrix: $(NAME)
	RUNS="$(MATRIX_RUNS)" bash ./scripts/edge_matrix.sh

stress:
	$(CC) $(CFLAGS) tests/stress_scheduler.c src/scheduler/scheduler.c src/scheduler/scheduler_sync.c src/scheduler/heap.c $(INCLUDES) -o $(STRESS_NAME)
	./$(STRESS_NAME)

norm:
	norminette src include

.PHONY: all clean fclean re run smoke consistency consistency-batches edge-matrix stress norm
