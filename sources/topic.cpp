#include "Server.hpp"

void server::do_topic(std::vector<std::string> &command, client &clt,
                      std::string line) {
  if (command.size() < 2 || command[1].at(0) != '#')
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  command[1].erase(0, 1);
  size_t i;
  for (i = 0; i < channels.size(); i++) {
    if (channels[i].getName().compare(command[1]) == 0)
      break;
  }
  if (i == channels.size())
    return send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  if (command.size() == 2) {
    // TOPIC #channelname
    if (channels[i].getTopic().empty())
      send_reply(clt.getFd(), RPL_NOTOPIC(clt.getNickname(), command[1]));
    else
      send_reply(clt.getFd(), RPL_TOPIC(clt.getNickname(), command[1], channels[i].getTopic()));
  }
  else if (command.size() > 2 && command[2].at(0) == ':') {
    // TOPIC #channelname :new topic
    if (!channels[i].c_modes[TOPIC_RESTRICTION_M] || channels[i].getOperatorIndex(clt.getNickname()) != NOT_VALID) {
      std::string new_topic = extract_param(command, line, 2);
      channels[i].setTopic(new_topic);
      channels[i].msgToAllMemebers(TOPIC_TO_ALL(clt.getNickname(), command[1], new_topic));
    } else
      send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
  }
}

