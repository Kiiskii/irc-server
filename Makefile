CXX = c++
FLAGS = -std=c++20

OBJS_DIR = obj
HDRS_DIR = inc/
SRCS_DIR = srcs/
HDRS = -I$(HDRS_DIR)

NAME = ircserv
# SRCS = $(wildcard $(SRCS_DIR)*.cpp)

SRCS = $(SRCS_DIR)main.cpp \
		$(SRCS_DIR)Server.cpp \
		$(SRCS_DIR)Client.cpp \
		$(SRCS_DIR)utils.cpp \
		$(SRCS_DIR)Join.cpp \
		$(SRCS_DIR)Topic.cpp \
		$(SRCS_DIR)Mode.cpp \
		$(SRCS_DIR)ModeHandlers.cpp \
		$(SRCS_DIR)Channel.cpp \
		$(SRCS_DIR)Invite.cpp \
		$(SRCS_DIR)pass.cpp \
		$(SRCS_DIR)nick.cpp \
		$(SRCS_DIR)user.cpp \
		$(SRCS_DIR)ping.cpp \
		$(SRCS_DIR)Message.cpp \
		$(SRCS_DIR)parsing.cpp \
		$(SRCS_DIR)Privmsg.cpp \
		$(SRCS_DIR)Kick.cpp

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
