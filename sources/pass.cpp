#include "Server.hpp"

void server::check_password(std::vector<std::string> &command, client &clt) {
  std::string reply;
  if (command.size() < 2)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  if (clt.authentication[0])
    send_reply(clt.getFd(), (ERR_ALREADYREGISTERED(clt.getNickname())));
  else {
    if (command[1].compare(password) == 0)
      clt.authentication[0] = true;
    else
      send_reply(clt.getFd(), (ERR_PASSWDMISMATCH(clt.getNickname())));
  }
}

