#include "Server.hpp"

void server::send_pong(std::vector<std::string> &command, client &clt) {
  if (command.size() < 2 || command[1].empty())
    send_reply(clt.getFd(),
               (ERR_NEEDMOREPARAMS(clt.getNickname(), command[0])));
  else
    send_reply(clt.getFd(),
               "PONG " + std::string(SERVER_NAME) + " " + command[1] + "\r\n");
}

