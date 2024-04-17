PROJECT	= ircserv

### COMPILATION ###
CC		= c++
CFLAGS	= -Wall -Wextra -std=c++98 -g3
CFLAGS	+= -MMD -MP
INCLUDE	= -I$(H_DIR) -I$(TEST_DIR)

### EXECUTABLE ###
NAME	= $(PROJECT)

PORT	= 6661


PASSWD	= password
ARGS	= $(PORT) $(PASSWD)

TEST	= $(OBJ_DIR)/test_bin

### ENV VARIABLES ###
-include .env
FDEBUG		?= false

ifeq ($(FDEBUG),true)
	CFLAGS += -g3
	VFLAGS += -D DEBUG_MODE
endif

### INCLUDES ###
OBJ_DIR		= bin
SRC_DIR		= src
TEST_DIR	= tests
H_DIR		= incl

### SOURCE FILES ###
SRCS	=	main.cpp \
			Server.cpp \
			Log.cpp \
			User.cpp \
			Channel.cpp \
			Message.cpp \
			utils/min.cpp \
			utils/Utils.cpp \
			utils/changeName.cpp \
			Commands/Join.cpp \
			Commands/Nick.cpp \
			Commands/Privmsg.cpp \
			Commands/Part.cpp \
			Commands/Ping-Pong.cpp \
			Commands/Kick.cpp \
			Commands/Topic.cpp \
			Commands/Mode.cpp \
			Commands/Invite.cpp \
			Commands/Notice.cpp \
			Commands/Quit.cpp \

### OBJECTS ###
OBJS	= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS	= $(OBJS:.o=.d)

### COLORS ###
RESET	= \033[0m
BLACK	= \034[1;30m
RED		= \033[1;31m
GREEN	= \033[1;32m
YELLOW	= \033[1;33m
BLUE	= \033[1;34m
PURPLE	= \033[1;35m
CYAN	= \033[1;36m
WHITE	= \033[1;37m

### OTHERS ### Utile pour utilise valgrind sur mac
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	VALGRIND	= leaks --list --groupByType --atExit --
else
	VALGRIND	= valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all
endif

### RULES ###
all:	$(NAME)

$(NAME):	$(OBJS)
	@$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) $(LINKS) -o $(NAME)
	@echo "$(PROJECT): $(BLUE)Creating program file -> $(WHITE)$(notdir $@)... $(GREEN)[Done]$(RESET)"
	@echo "$(PROJECT): $(GREEN)Project successfully compiled$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDE) $(VFLAGS) -c $< -o $@
	@echo "$(PROJECT): $(BLUE)Creating object file -> $(WHITE)$(notdir $@)... $(GREEN)[Done]$(RESET)"

run: $(NAME)
	@./$(NAME) $(ARGS)

val: $(NAME)
	@$(VALGRIND) ./$(NAME) $(ARGS)

info:
	@echo "$(BLUE)NAME$(RESET): $(NAME)"
	@echo "$(BLUE)CFLAGS$(RESET): $(CFLAGS)"
	@echo "$(BLUE)INCLUDE$(RESET): $(INCLUDE)"
	@echo "$(BLUE)LFLAGS$(RESET): $(LFLAGS)"
	@echo "$(BLUE)LINKS$(RESET): $(LINKS)"
	@echo "$(BLUE)SRCS$(RESET): $(SRCS)"
	@echo "$(BLUE)T_OBJS$(RESET): $(T_OBJS)"

clean:
	@echo "$(PROJECT): $(RED)Supressing object files$(RESET)"
	@rm -rf $(OBJ_DIR)

##utile pour les tests statiques pour checker des erreurs de norme ou synthax
lint:
	@/usr/bin/clang-tidy-12 src/*.cpp src/*/*.cpp incl/*.h

fclean:	clean
	@echo "$(PROJECT): $(RED)Supressing program file$(RESET)"
	@rm -f $(NAME)

re:	fclean
	@$(MAKE) all

.PHONY:	all clean fclean re test info check run val

-include $(DEPS)
