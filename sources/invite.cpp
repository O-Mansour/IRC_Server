#include "Server.hpp"

void server::do_invite(std::vector<std::string> &command, client &clt) {
  if (command.size() != 3)
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  bool user_exist = false;
  int fd;
  command[2].erase(0, 1);
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getNickname().compare(command[1]) == 0) {
      user_exist = true;
      fd = clients[i]->getFd();
    } else if (this->bot.getNickname().compare(command[1]) == 0) // checking also for bot name
      user_exist = true;
  }
  if (!user_exist)
    return send_reply(clt.getFd(), ERR_NOSUCHNICK(clt.getNickname(), command[1]));
  size_t i = 0;
  for (; i < channels.size(); i++) {
    if (channels[i].getName().compare(command[2]) == 0 && channels[i].getSize() > 0)
      break ;
  }
  if (i == channels.size())
    return send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[2]));

  if (this->bot.getNickname().compare(command[1]) == 0) // adding the bot to the channel
  {
    if (!channels[i].getIsBotJoined())
      channels[i].setIsBotJoined(true);
    return;
  }
  if (channels[i].check_nickname(command[1]))
    return send_reply(clt.getFd(), ERR_USERONCHANNEL(clt.getNickname(), command[1], command[2]));
  else if (!channels[i].check_nickname(clt.getNickname()))
    return send_reply(clt.getFd(), ERR_NOTONCHANNEL(clt.getNickname(), command[2]));
  else if (channels[i].c_modes[INVITE_ONLY_M] && channels[i].getOperatorIndex(clt.getNickname()) == -1)
    return send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[2]));
  send_reply(fd, RPL_INVITING(clt.getNickname(), command[1], command[2]));
  channels[i].addToInvitedFds(fd);
}
