NAME = minesweeper

.PHONY: all clean fclean re

all: $(NAME)

$(NAME):
	@printf "%-50s" "create executable "$(notdir $@)...
	@gcc -o $(NAME) minesweeper.c SDLMain.m -I SDL.framework/Headers\
		-L SDL.framework/Versions/Current -F . -framework SDL -framework Cocoa -m32
	@printf "\e[32m[OK]\e[0m\n"
	@printf "%-50s" "changing lib path..."
	@install_name_tool -change @executable_path/../Frameworks/SDL.framework/SDL @executable_path/SDL.framework/SDL $(NAME)
	@printf "\e[32m[OK]\e[0m\n"

clean:

fclean: clean
	@printf "%-50s" "deleting executable..." 
	@$(RM) $(NAME)
	@printf "\e[32m[OK]\e[0m\n"

re: fclean all
