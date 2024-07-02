#include "Server.hpp"

void server::do_join(std::vector<std::string> &command, client *clt) {
  if (command.size() < 2)
    return send_reply(clt->getFd(),
                      ERR_NEEDMOREPARAMS(clt->getNickname(), command[0]));
  // "JOIN 0" to leave all channels
  if (command[1].compare("0") == 0) {
    for (size_t i = 0; i < channels.size(); i++) {
      int uIndex = channels[i].getUserIndex(clt->getNickname());
      if (uIndex != NOT_VALID) {
        channels[i].remove_user(uIndex, clt->getNickname());
        if (channels[i].getSize() == 0)
          channels.erase(channels.begin() + i);
        send_reply(clt->getFd(), RPL_PART(clt->getNickname(), channels[i].getName(), std::string("Leaving all channels")));
      }
    }
    return;
  }
  std::vector<std::string> chan_list = split_line(command[1], ",");
  std::vector<std::string> keys_list;
  if (command.size() > 2)
    keys_list = split_line(command[2], ",");
  for (size_t i = 0; i < chan_list.size(); i++) {
    if (chan_list[i].at(0) != '#' || chan_list[i].length() <= 1) {
      send_reply(clt->getFd(), ERR_NEEDMOREPARAMS(clt->getNickname(), command[0]));
      continue;
    }
    chan_list[i].erase(0, 1);
    std::string c_key = "";
    if (keys_list.size() > i)
      c_key = keys_list[i];
    size_t j;
    for (j = 0; j < channels.size(); j++) {
      if (channels[j].getName().compare(chan_list[i]) == 0) {
        // check channel modes
        if (channels[j].c_modes[INVITE_ONLY_M] && channels[j].getInviteIndex(clt->getFd()) == NOT_VALID)
          send_reply(clt->getFd(), ERR_INVITEONLYCHAN(clt->getNickname(), chan_list[i]));
        else if (channels[j].c_modes[USER_LIMIT_M] && channels[j].getSize() >= channels[j].getUserLimit())
          send_reply(clt->getFd(), ERR_CHANNELISFULL(clt->getNickname(), chan_list[i]));
        else
        {
          channels[j].c_join(clt, c_key);
          int invite_idx = channels[j].getInviteIndex(clt->getFd());
          if (invite_idx != NOT_VALID)
            channels[j].removeFromInvitedFds(invite_idx);
        }
        break;
      }
    }
    if (j == channels.size()) {
      // create channel and add the user to it
      if (chan_list[i].find(",") != std::string::npos || chan_list[i].find("\a") != std::string::npos)
        return;
      channel chnl(chan_list[i], clt);
      chnl.c_join(clt, "");
      channels.push_back(chnl);
    }
  }
}

