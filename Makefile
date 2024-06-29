NAME = ircserv

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes

CXX= c++

SRCS  = main.cpp sources/Server.cpp sources/Client.cpp sources/Channel.cpp\
				sources/Bot.cpp sources/Bot_utils.cpp sources/Dcc.cpp sources/invite.cpp\
				sources/join.cpp sources/kick.cpp sources/mode.cpp sources/nick.cpp\
				sources/pass.cpp sources/pong.cpp sources/privmsg.cpp sources/topic.cpp\
				sources/user.cpp sources/utils.cpp

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean :
	rm -rf $(OBJS)

bear: compile_commands.json

compile_commands.json:
	bear -- make

fclean : clean
	rm -fr $(NAME)

re : fclean all

run :
	make && make clean > /dev/null && clear && ./$(NAME) 6667 ok
