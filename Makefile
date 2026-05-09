NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I include

SRCS		= src/main.c \
			src/args.c \
			src/sim.c \
			src/coder.c

ARGS		?= 5 800 200 200 200 3 0 fifo

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

bonus: all

run: $(NAME)
	./$(NAME) $(ARGS)

.PHONY: all clean fclean re bonus run
