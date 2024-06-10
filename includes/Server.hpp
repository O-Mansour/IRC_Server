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
  void send_reply(int fd, std::string str);
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
  void do_kick(std::vector<std::string> &command, client &clt);
  void do_mode(std::vector<std::string> &command, client &clt);
  void send_pong(std::vector<std::string> &command, client &clt);
  void register_user(client &clt);
};

void print_time();
std::string print_time_welcome();
void print_ft_irc();
