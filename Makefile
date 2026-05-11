NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I include

SRCS		= src/main.c \
			src/args.c \
			src/context.c \
			src/context_scheduler.c \
			src/coder.c \
			src/coder_scheduler.c \
			src/logger.c \
			src/monitor.c \
			src/state.c \
			src/sleep.c \
			src/scheduler.c \
			src/scheduler_sync.c \
			src/heap.c \
			src/utils.c \

# Arguments:
# <number_of_coders>
# <time_to_burnout>
# <time_to_compile>
# <time_to_debug>
# <time_to_refactor>
# <number_of_compiles_required>
# <dongle_cooldown>
# <scheduler>
ARGS		?= 5 800 200 200 200 3 50 fifo

OBJS		= $(SRCS:src/%.c=obj/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

# run with short timeouts for testing:
# make run ARGS="5 100 20 20 20 3 10 fifo"
run: $(NAME)
	./$(NAME) $(ARGS)


norm:
	norminette src include

.PHONY: all clean fclean re run norm
