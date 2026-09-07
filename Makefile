NAME	= 	codexion
CC	= 	cc
CFLAGS	= 	-Wall -Werror -Wextra -pthread
RM		= rm -f

GREEN	= \033[0;32m
CYAN	= \033[0;36m
PURPLE	= \033[0;35m
RESET	= \033[0m

SRC	=	coder.c			\
		cleanup.c		\
		codexion.c		\
		dongle.c		\
		heap.c			\
		init.c			\
		logging.c		\
		monitor.c		\
		parsing.c		\
		release.c		\
		scheduler.c		\
		simulation.c	\
		sync.c
		
OBJ	= 	$(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)codexion successfully created!$(RESET)"

%.o: %.c codexion.h
	@$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	@$(RM) $(OBJ)
	@echo "$(CYAN)Object files removed.$(RESET)"

fclean: clean
	@$(RM) $(NAME)
	@echo "$(PURPLE)Program removed.$(RESET)"

re: fclean all

.PHONY: all clean fclean re
