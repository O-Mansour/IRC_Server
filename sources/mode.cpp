#include "Server.hpp"

long isValidInteger(const std::string &str) {
  if (str.empty() || str.length() > 10)
    return 0;
  for (size_t i = 0; i < str.length(); i++) {
    if (!isdigit(str[i]))
      return 0;
  }
  long res = std::atol(str.c_str());
  if (res > INT_MAX || res < 0)
    return 0;
  return res;
}


void server::do_mode(std::vector<std::string> &command, client &clt) {
  if (command.size() < 2)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  // user's modes aren't supported
  else if (command[1].at(0) != '#')
    return send_reply(clt.getFd(), ERR_UMODEUNKNOWNFLAG(clt.getNickname()));
  command[1].erase(0, 1);
  if (command[1].empty())
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  // get the channel index
  size_t i;
  for (i = 0; i < channels.size(); i++) {
    if (channels[i].getName().compare(command[1]) == 0)
      break;
  }
  if (i == channels.size())
    return send_reply(clt.getFd(),
                      ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  if (command.size() == 3) {
    if (channels[i].getOperatorIndex(clt.getNickname()) == -1)
      return send_reply(clt.getFd(),
                        ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
    // cases : +i -i +t -t -k -l
    if (command[2].compare("+i") == 0)
      channels[i].c_modes[INVITE_ONLY_M] = true;
    else if (command[2].compare("-i") == 0)
      channels[i].c_modes[INVITE_ONLY_M] = false;
    else if (command[2].compare("+t") == 0)
      channels[i].c_modes[TOPIC_RESTRICTION_M] = true;
    else if (command[2].compare("-t") == 0)
      channels[i].c_modes[TOPIC_RESTRICTION_M] = false;
    else if (command[2].compare("-k") == 0) {
      channels[i].setKey("");
      channels[i].c_modes[CHANNEL_KEY_M] = false;
    } else if (command[2].compare("-l") == 0) {
      channels[i].setUserLimit(0);
      channels[i].c_modes[USER_LIMIT_M] = false;
    }
  } else if (command.size() == 4) {
    if (channels[i].getOperatorIndex(clt.getNickname()) == -1)
      return send_reply(clt.getFd(),
                        ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
    // cases : +k +l +o -o
    if (command[2].compare("+k") == 0) {
      channels[i].setKey(command[3]);
      channels[i].c_modes[CHANNEL_KEY_M] = true;
    } else if (command[2].compare("+l") == 0) {
      // checking is the number valid
      long num = isValidInteger(command[3]);
      if (num != 0) {
        channels[i].setUserLimit(std::atoi(command[3].c_str()));
        channels[i].c_modes[USER_LIMIT_M] = true;
      }
    } else if (command[2].compare("+o") == 0) {
      int clt_index = channels[i].getUserIndex(command[3]);
      if (clt_index != NOT_VALID)
      {
        channels[i].addAsOperator(clt_index);
        channels[i].msgToAllMemebers(RPL_MODE(channels[i].getName(), "+o", command[3]));
      }
      return ;
    } else if (command[2].compare("-o") == 0) {
      int op_index = channels[i].getOperatorIndex(command[3]);
      if (op_index != NOT_VALID)
      {
        channels[i].eraseOperator(op_index);
        channels[i].msgToAllMemebers(RPL_MODE(channels[i].getName(), "-o", command[3]));
      }
      return ;
    }
  }
  std::string modes;
  if (channels[i].c_modes[INVITE_ONLY_M])
    modes += "i";
  if (channels[i].c_modes[TOPIC_RESTRICTION_M])
    modes += "t";
  if (channels[i].c_modes[CHANNEL_KEY_M])
    modes += "k";
  if (channels[i].c_modes[USER_LIMIT_M])
    modes += "l";
  return send_reply(clt.getFd(),
                    RPL_CHANNELMODEIS(clt.getNickname(), command[1], modes));
}

