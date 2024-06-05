NAME = ircserv

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes

CXX= c++

SRCS  = main.cpp sources/Server.cpp sources/Client.cpp sources/Channel.cpp\

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -fr $(NAME)

re : fclean all

run :
	make && make clean > /dev/null && clear && echo "Compiled successufly..."
