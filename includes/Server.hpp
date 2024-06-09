#pragma once

//=========Includes=============
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include "Client.hpp"
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include "Channel.hpp"

//=========Errors===============
//PASS :
# define ERR_NEEDMOREPARAMS() (RED "localhost: Not enough parameters\r\n" RESET)
# define ERR_ALREADYREGISTERED(client) (RED "localhost: " + client + " :You may not reregister\r\n" RESET)
# define ERR_PASSWDMISMATCH() (RED "localhost: Password incorrect\r\n" RESET)
//NICK :
# define ERR_NONICKNAMEGIVEN(client) (RED "localhost: No nickname given\r\n" RESET)
# define ERR_ERRONEUSNICKNAME()(RED "localhost: Invalid character\r\n" RESET)
# define ERR_NICKNAMEINUSE(client, nick)(RED "localhost: " + client + " This \"" + nick + "\" Nickname is already in use\r\n" RESET)
# define ERR_NICKNAMECHANGE(client, nick)(GREEN "localhost: " + client + " Changed his nickname to " + nick + " \r\n" RESET)
# define ERR_NICKNAMEFIRST()(RED "localhost: Enter a nickname first\r\n" RESET)
# define ERR_NICKNAMEINVALID(client)(RED "localhost: " + client + " :Is invalid\r\n" RESET)

//USER
#define ERR_USERFORMAT() (UNDERLINE "localhost: Please set the second parameter with <0> and the third with <*>\r\n" RESET)
#define ERR_USERSYNTAX() (RED "localhost: Please use this syntax : USER <username> 0 * :<realname>\r\n" RESET)



//==========Macros===============
#define SERVER_NAME "EKIP.EKIP"
#define BUFFER_SIZE 512
#define NOT_VALID   -1
#define MAX_UINT16 65535
#define RESERVED_PORTS 1024

//bunch of colors and stuff
#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define UNDERLINE "\033[4;37m"
#define BOLD "\033\e[1m"

//==========Classes==============
class server {
	private:
		int port;
		std::string password;
		int s_socket;
		std::vector <struct pollfd> poll_fds;
		std::vector <client> clients;
		std::map<int, std::string> read_buffer;
		std::vector <channel> channels;
	public:
		server(int p, std::string pass);
		~server();
		void start();
		void CreateClient();
		void HandleData(client& clt);
		void deleteClientData(client& clt);
		void execute_cmds(client& clt);
		void authenticate_cmds(std::string line, client& clt);
		void check_password(std::vector<std::string> &command, client& clt);
		void send_reply(int fd, std::string str);
		void check_nickname(std::vector<std::string> &command, client& clt);
		void check_username(std::vector<std::string>& command, client& clt, std::string &line);
		void channel_cmds(std::string line, client& clt);
		void do_join(std::vector<std::string> &command, client& clt);
		void do_privmsg(std::vector<std::string> &command, client &clt, std::string line);
		void do_topic(std::vector<std::string> &command, client& clt, std::string line);
		void do_invite(std::vector<std::string> &command, client& clt);
		void do_kick(std::vector<std::string> &command, client& clt);
		void do_mode(std::vector<std::string> &command, client &clt);
};

void print_time();
void print_ft_irc();
