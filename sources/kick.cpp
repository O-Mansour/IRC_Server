#include "Server.hpp"

void server::do_kick(std::vector<std::string> &command, client &clt,
                     std::string line) {
  std::stringstream ss;
  if (command.size() < 4)
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  else {
    command[1].erase(0, 1);
    size_t i = 0;
    for (; i < channels.size(); i++) {
      if (channels[i].getName().compare(command[1]) == 0) {
        if (channels[i].getUserIndex(clt.getNickname()) == NOT_VALID)
          return send_reply(clt.getFd(), ERR_NOTONCHANNEL(clt.getNickname(), command[1]));
        else if (channels[i].getOperatorIndex(clt.getNickname()) == NOT_VALID)
          return send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
        int j = channels[i].getUserIndex(command[2]);
        if (j == NOT_VALID)
          return send_reply(clt.getFd(), ERR_USERNOTINCHANNEL(clt.getNickname(), command[2], command[1]));
        std::string reason;
        reason = extract_param(command, line, 3);
        if (reason.empty()) {
          reason = "Operator didn't tell why";
          channels[i].msgToAllMemebers(RPL_KICK(clt.getNickname(), command[1], command[2], reason));
        }
        else
          channels[i].msgToAllMemebers(RPL_KICK(clt.getNickname(), command[1], command[2], reason));
        channels[i].remove_user(j, command[2]);
        if (channels[i].getSize() == 0)
          channels.erase(channels.begin() + i);
        return ;
      }
    }
    if (i == channels.size())
      send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  }
}

