NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror
INCLUDES	= -I include

SRCS		= src/main.c \
			src/args.c

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

.PHONY: all clean fclean re bonus
