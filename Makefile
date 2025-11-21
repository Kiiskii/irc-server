CXX = c++
FLAGS = -std=c++20

OBJS_DIR = obj
HDRS_DIR = inc/
HDRS = -I$(HDRS_DIR)

NAME = ircserv
SRCS = srcs/main.cpp srcs/Server.cpp srcs/Client.cpp  srcs/utils.cpp srcs/Join.cpp srcs/Topic.cpp srcs/Mode.cpp srcs/ModeHandlers.cpp srcs/Channel.cpp srcs/Invite.cpp \
srcs/pass.cpp srcs/nick.cpp srcs/user.cpp srcs/ping.cpp srcs/Message.cpp

OBJS = $(patsubst srcs/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))

GREEN = \033[32m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Compiling executable...$(RESET)"
	@$(CXX) $(FLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)COMPILATION COMPLETE!$(RESET)"
	@echo "Usage: ./ircserv <port> <set_password>"

$(OBJS_DIR)/%.o: srcs/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(FLAGS) $(HDRS) -MMD -MP -c $< -o $@

clean:
	@echo "$(GREEN)Cleaning object files$(RESET)"
	rm -rf $(OBJS_DIR)

fclean: clean
	@echo "$(GREEN)Removing executable$(RESET)"
	rm -f $(NAME)

re: fclean all

-include $(OBJS:.o=.d)

.SECONDARY: $(OBJS)

.PHONY: all clean fclean re
