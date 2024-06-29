#include "Server.hpp"

void server::register_user(client &clt) {
  send_reply(clt.getFd(), RPL_WELCOME(clt.getNickname(), clt.getNickname()));
  send_reply(clt.getFd(), RPL_YOURHOST(clt.getNickname()));
  send_reply(clt.getFd(), RPL_CREATED(clt.getNickname()));
  send_reply(clt.getFd(), RPL_MYINFO(clt.getNickname()));
  send_reply(clt.getFd(), RPL_ISUPPORT(clt.getNickname()));
}

void server::check_username(std::vector<std::string> &command, client &clt,
                            std::string &line) {
  if (command.size() < 5)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  else if (clt.authentication[2])
    return send_reply(clt.getFd(), ERR_ALREADYREGISTERED(clt.getNickname()));
  else if (command[2].compare("0") != 0 || command[3].compare("*") != 0)
    return send_reply(clt.getFd(), ERR_USERFORMAT());
  else if (command[4].at(0) != ':' || command[4].length() < 2)
    return send_reply(clt.getFd(), ERR_USERSYNTAX());
  else {
    clt.setUsername(command[1]);
    line = extract_param(command, line, 4);
    clt.setFullname(line);
    clt.authentication[2] = true;
    register_user(clt);
  }
}

