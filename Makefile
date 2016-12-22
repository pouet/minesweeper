SRC_NAME = $(sort minesweeper.c)
SRC_PATH = ./src/
OBJ_PATH = ./obj/
INC_PATH = ./includes/
OBJ_NAME = $(SRC_NAME:.c=.o)

SRC = $(addprefix $(SRC_PATH),$(SRC_NAME))
OBJ = $(addprefix $(OBJ_PATH),$(OBJ_NAME))
INC = $(addprefix -I,$(INC_PATH))

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O3
NAME = minesweeper

UNAME_S = $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	SDL_CONFIG = `sdl-config --cflags --libs`
	INSTALL_NAME_TOOL =
else # Mac OS
	SRC_NAME += SDLMain.m
	LDFLAGS += -L ./SDL.framework/Versions/Current
	INC_PATH += SDL.framework/Headers
	SDL_CONFIG = -F . -framework SDL
	INSTALL_NAME_TOOL = install_name_tool -change @rpath/SDL.framework/Versions/A/SDL\
											@executable_path/SDL.framework/SDL $(NAME)
endif


.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJ)
	@printf "%-50s" "create executable "$(notdir $@)...
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(OBJ) -o $(NAME) $(SDL_CONFIG)
	@$(INSTALL_NAME_TOOL)
	@printf "\033[1;32m[OK]\033[0m\n"

$(OBJ_PATH)%.o: $(SRC_PATH)%.c
	@printf "%-50s" "compiling "$(notdir $<)...
	@mkdir -p $(OBJ_PATH)
	@$(CC) $(CFLAGS) $(INC) -o $@ -c $< $(SDL_CONFIG)
	@printf "\033[1;32m[OK]\033[0m\n"

clean:
	@printf "%-50s" "deleting objects..." 
	@$(RM) $(OBJ)
	@$(RM) -r $(OBJ_PATH)
	@printf "\033[1;32m[OK]\033[0m\n"

fclean: clean
	@printf "%-50s" "deleting executable..." 
	@$(RM) $(NAME)
	@printf "\033[1;32m[OK]\033[0m\n"

re: fclean all
