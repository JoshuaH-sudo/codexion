NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I include

SRCS		= src/main.c \
			src/args.c \
			src/context.c \
			src/coder.c \
			src/logger.c \
			src/monitor.c \
			src/state.c \

# Arguments:
# <num_coders>
# <compile_time_ms>
# <compile_time_variance_ms>
# <rest_time_ms>
# <rest_time_variance_ms>
# <num_compilations>
# <monitor_interval_ms>
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

run: $(NAME)
	./$(NAME) $(ARGS)

norm:
	norminette src include

.PHONY: all clean fclean re run norm
