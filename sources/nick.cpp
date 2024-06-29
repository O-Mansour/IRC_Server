#include "Server.hpp"

int ValidNick(std::string &str) {
  if (str[0] == '#' || str[0] == '$' || str[0] == '&' || str[0] == ':')
    return 0;
  for (size_t i = 0; i < str.length(); i++) {
    if ((str[i] >= 9 && str[i] <= 13) || str[i] == 32 || str[i] == ',' ||
        str[i] == '*' || str[i] == '?' || str[i] == '!' || str[i] == '@' ||
        str[i] == '.')
      return 0;
  }
  return 1;
}

void server::check_nickname(std::vector<std::string> &command, client &clt) {
  std::string reply;
  if (command.size() != 2)
    send_reply(clt.getFd(), ERR_NONICKNAMEGIVEN(clt.getNickname()));
  else if (!ValidNick(command[1]))
    send_reply(clt.getFd(),
               ERR_ERRONEUSNICKNAME(clt.getNickname(), command[1]));
  else if (command[1].compare(this->bot.getNickname()) == 0) {
    send_reply(clt.getFd(), ERR_NICKNAMEINUSE(clt.getNickname(), command[1]));
  } else {
    size_t i;
    for (i = 0; i < clients.size(); i++) {
      if (clients[i]->getNickname().compare(command[1]) == 0) {
        send_reply(clt.getFd(),
                   ERR_NICKNAMEINUSE(clt.getNickname(), command[1]));
        break;
      }
    }
    if (i == clients.size()) {
      if (clt.authentication[1]){
        for (size_t j = 0; j < clients.size(); j++)
					if (clients[j]->getFd() != clt.getFd())
          	send_reply(clients[j]->getFd(), RPL_NICK(clt.getNickname(), command[1]));
      }
      clt.setNickname(command[1]);
      clt.authentication[1] = true;
    }
  }
}

