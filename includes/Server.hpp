#pragma once

//=========Includes=============
#include "Bot.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "errors.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include "Channel.hpp"
#include <limits.h>

//=========Errors===============
//Welcome
// # define RPL_WELCOME(nickname) (":localhost 001 " + nickname + " :Welcome to the Internet Relay Network \r\n")
// # define RPL_YOURHOST() (":localhost 002  :Your host is  (localhost), running version \r\n")
// # define RPL_CREATED() (":localhost 003 :This server was created \r\n")
// # define RPL_MYINFO() (":localhost 004 \r\n")
// # define RPL_ISUPPORT() (":localhost 005 :are supported by this server\r\n")
// //PASS :
// # define ERR_NEEDMOREPARAMS() (RED "localhost: Not enough parameters\r\n" RESET)
// # define ERR_ALREADYREGISTERED(client) (RED "localhost: " + client + " :You may not reregister\r\n" RESET)
// # define ERR_PASSWDMISMATCH() (RED "localhost: Password incorrect\r\n" RESET)
// //NICK :
// # define ERR_NONICKNAMEGIVEN(client) (RED "localhost: No nickname given\r\n" RESET)
// # define ERR_ERRONEUSNICKNAME()(RED "localhost: Invalid character\r\n" RESET)
// # define ERR_NICKNAMEINUSE(client, nick)(RED "localhost: " + client + " This \"" + nick + "\" Nickname is already in use\r\n" RESET)
// # define ERR_NICKNAMECHANGE(client, nick)(GREEN "localhost: " + client + " Changed his nickname to " + nick + " \r\n" RESET)
// # define ERR_NICKNAMEFIRST()(RED "localhost: Enter a nickname first\r\n" RESET)
// # define ERR_NICKNAMEINVALID(client)(RED "localhost: " + client + " :Is invalid\r\n" RESET)

// //USER
// #define ERR_USERFORMAT() (UNDERLINE "localhost: Please set the second parameter with <0> and the third with <*>\r\n" RESET)
// #define ERR_USERSYNTAX() (RED "localhost: Please use this syntax : USER <username> 0 * :<realname>\r\n" RESET)

//JOIN :
# define ERR_BADCHANNELKEY(nick, channel) (":localhost 475 " + nick + " #" + channel + " :Cannot join channel (+k)\r\n")
# define RPL_JOIN(nick, channel) (":" + nick + "@localhost JOIN :#" +  channel + "\r\n")
# define RPL_NAMREPLY(nick, channel, nicks_list) (":localhost 353 " + nick + " = #" + channel + " :" + nicks_list + "\r\n")
# define RPL_ENDOFNAMES(nick, channel) (":localhost 366 " + nick + " #" + channel + " :End of /NAMES list\r\n")
# define ERR_INVITEONLYCHAN(nick, channel) (":localhost 473 " + nick + " #" + channel + " :Cannot join channel (+i)\r\n")
# define ERR_CHANNELISFULL(nick, channel) (":localhost 471 " + nick + " #" + channel + " :Cannot join channel (+l)\r\n")

// TOPIC
# define RPL_TOPIC(nick, channel, topic) (":localhost 332 " + nick + " #" + channel + " :" + topic + "\r\n")
# define RPL_NOTOPIC(nick, channel) (":localhost 331 " + nick + " #" + channel + " :No topic is set\r\n")

// MODE
# define ERR_UMODEUNKNOWNFLAG(nick) (":localhost 501 " + nick + " :Unknown MODE flag\r\n")
# define RPL_CHANNELMODEIS(nick, channel, modes) (":localhost 324 " + nick + " #" + channel + " +" + modes + "\r\n")

# define ERR_UNKNOWNCOMMAND(nick, command) (":localhost 421 " + nick + " " + command + " :Unknown command\r\n")

//==========Macros===============
#define SERVER_NAME "FT_IRC"
#define BUFFER_SIZE 512
#define NOT_VALID -1
#define MAX_UINT16 65535
#define RESERVED_PORTS 1024

// bunch of colors and stuff
#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define UNDERLINE "\033[4;37m"
#define BOLD "\033\e[1m"

class channel;

//==========Classes==============
class server {
private:
  int port;
  std::string password;
  int s_socket;
  std::vector<struct pollfd> poll_fds;
  std::vector<client> clients;
  std::map<int, std::string> read_buffer;
  std::vector<channel> channels;

  /* adding the bot so the server can send message to the bot directly; */
  Bot bot;

public:
  server(int p, std::string pass);
  ~server();
  void start();
  void CreateClient();
  void HandleData(client &clt);
  void deleteClientData(client &clt);
  void execute_cmds(client &clt);
  void authenticate_cmds(std::string line, client &clt);
  void check_password(std::vector<std::string> &command, client &clt);
  void check_nickname(std::vector<std::string> &command, client &clt);
  void check_username(std::vector<std::string> &command, client &clt,
                      std::string &line);
  void channel_cmds(std::string line, client &clt);
  void do_join(std::vector<std::string> &command, client &clt);
  void do_privmsg(std::vector<std::string> &command, client &clt,
                  std::string line);
  void do_topic(std::vector<std::string> &command, client &clt,
                std::string line);
  void do_invite(std::vector<std::string> &command, client &clt);
  void do_kick(std::vector<std::string> &command, client &clt, std::string line);
  void do_mode(std::vector<std::string> &command, client &clt);
  void send_pong(std::vector<std::string> &command, client &clt);
  void register_user(client &clt);
};
void send_reply(int fd, std::string str);

void print_time();
std::string print_time_welcome();
void print_ft_irc();
