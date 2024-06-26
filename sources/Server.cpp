#include "../includes/Server.hpp"
#include "../includes/Bot.hpp"
#include "Dcc.hpp"
#include "errors.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

server::server(int p, std::string pass)
    : port(p), password(pass), bot("lhaj", "lhaj Molshi", "lhaj") {
  if ((this->s_socket = socket(AF_INET, SOCK_STREAM, 0)) == NOT_VALID)
    throw std::runtime_error("socket() failed");
  struct sockaddr_in s_addr;
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(port);
  s_addr.sin_addr.s_addr = INADDR_ANY;
  int optval = 1;
  setsockopt(this->s_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == NOT_VALID)
    throw std::runtime_error("bind() failed");
  if (listen(this->s_socket, SOMAXCONN) == NOT_VALID)
    throw std::runtime_error("listen() failed");
  struct pollfd to_poll;
  to_poll.fd = this->s_socket;
  to_poll.events = POLLIN;
  this->poll_fds.push_back(to_poll);
}

server::~server() {}

void server::CreateClient() {
  struct sockaddr_in clt_addr;
  socklen_t addr_len = sizeof(clt_addr);
  int nb = accept(s_socket, (sockaddr *)&clt_addr, &addr_len);
  if (nb == NOT_VALID)
    throw std::runtime_error("accept() failed");
  struct pollfd clt_fd;
  clt_fd.fd = nb;
  clt_fd.events = POLLIN;
  poll_fds.push_back(clt_fd);
  client* clt = new client(nb);
  clients.push_back(clt);
  print_time();
  std::cout << YELLOW << "New client connected with fd : " << nb << RESET << std::endl;
}

void server::deleteClientData(client *clt) {
  print_time();
  std::cout << RED << "Client with fd : " << clt->getFd() << " disconnected" << RESET << std::endl;
  // remove him from all channels
  for (size_t i = 0; i < channels.size(); i++) {
    int uIndex = channels[i].getUserIndex(clt->getNickname());
    if (uIndex != NOT_VALID) {
      channels[i].remove_user(uIndex, clt->getNickname());
      if (channels[i].getSize() == 0)
        channels.erase(channels.begin() + i);
    }
  }
  // erase his read_buffer
  std::map<int, std::string>::iterator it = read_buffer.find(clt->getFd());
  if (it != read_buffer.end())
    read_buffer.erase(it);
  // erase him from poll_fds vector
  for (size_t i = 0; i < poll_fds.size(); i++) {
    if (poll_fds[i].fd == clt->getFd())
      poll_fds.erase(poll_fds.begin() + i);
  }
  // erase him from clients vector
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getFd() == clt->getFd())
      clients.erase(clients.begin() + i);
  }
  send_reply(clt->getFd(), RPL_QUIT(clt->getNickname(), "left the server"));
  close(clt->getFd());
  delete clt;
}

void send_reply(int fd, std::string str) {
  send(fd, str.c_str(), str.length(), 0);
}

void server::HandleData(client *clt) {
  char buf[BUFFER_SIZE];
  ssize_t rn = recv(clt->getFd(), buf, BUFFER_SIZE - 1, 0);
  if (rn < 0)
    throw std::runtime_error("recv() failed");
  else if (rn == 0)
    deleteClientData(clt);
  else {
    buf[rn] = '\0';
    read_buffer[clt->getFd()].append(buf);
    execute_cmds(clt);
  }
}

void server::start() {
  while (true) {
    if (poll(&poll_fds[0], poll_fds.size(), -1) == NOT_VALID)
      throw std::runtime_error("poll() failed");
    for (size_t i = 0; i < poll_fds.size(); i++) {
      if ((poll_fds[i].revents & POLLIN)) {
        if (poll_fds[i].fd == s_socket)
          this->CreateClient();
        else
          this->HandleData(clients[i - 1]);
      }
    }
  }
}

std::string extract_param(std::vector<std::string> &command, std::string line,
                          int argIndex) {
  size_t pos;
  for (int i = 0; i < argIndex; i++) {
    pos = line.find(command[i]);
    if (pos != std::string::npos)
      line.erase(0, pos + command[i].length());
  }
  pos = line.find(':');
  if (pos != std::string::npos)
    line.erase(0, pos + 1);
  return line;
}

std::vector<std::string> split_line(std::string line, std::string del) {
  std::vector<std::string> res;
  char *buff = new char[line.size() + 1];
  char *word;

  buff = std::strcpy(buff, line.c_str());
  word = std::strtok(buff, del.c_str());
  while (word) {
    res.push_back(word);
    word = std::strtok(NULL, del.c_str());
  }
  delete[] buff;
  return res;
}

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
      for (size_t j = 0; j < clients.size(); j++)
        send_reply(clients[j]->getFd(), RPL_NICK(clt.getNickname(), command[1]));
      clt.setNickname(command[1]);
      clt.authentication[1] = true;
    }
  }
}

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

void server::authenticate_cmds(std::string line, client &clt) {
  std::string reply;
  std::vector<std::string> command = split_line(line, " \t");
  if (command[0].compare("PASS") == 0)
    return check_password(command, clt);
  if (command[0].compare("NICK") == 0 && clt.authentication[0])
    return check_nickname(command, clt);
  else if (command[0].compare("NICK") == 0)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  if (command[0].compare("USER") == 0 && clt.authentication[0] &&
      clt.authentication[1])
    return check_username(command, clt, line);
  else if (command[0].compare("USER") == 0)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
}

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
        send_reply(clt->getFd(), RPL_PART(clt->getNickname(), channels[i].getName(), std::string("Good bye")));
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
        if (channels[j].c_modes[INVITE_ONLY_M])
          send_reply(clt->getFd(), ERR_INVITEONLYCHAN(clt->getNickname(), chan_list[i]));
        else if (channels[j].c_modes[USER_LIMIT_M] && channels[j].getSize() == channels[j].getUserLimit())
          send_reply(clt->getFd(), ERR_CHANNELISFULL(clt->getNickname(), chan_list[i]));
        else
          channels[j].c_join(clt, c_key);
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

void server::do_privmsg(std::vector<std::string> &command, client &clt,
                        std::string line) {
  if (command.size() < 3)
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  else {
    size_t i;
    if (command[1].at(0) == '#') {
      command[1].erase(0, 1);
      line = extract_param(command, line, 2);
      for (i = 0; i < channels.size(); i++) {
        if (channels[i].getName().compare(command[1]) == 0) {
          channels[i].c_privmsg(clt, line);
          // TODO: if the bot joined channel. check forward messages to bot
          if (channels[i].getIsBotJoined()) {
            this->bot.setMessage(line);
            std::string msg = this->bot.getResponse(channels[i].getName());
            if (msg.compare("-1") != 0 && msg.find("-2") == std::string::npos) {
              std::cout << "msg => :" << msg << std::endl;
              channels[i].msgToAllMemebers(msg);
            }
            if (msg.find("-2") != std::string::npos) {
              std::cout << "kicked by => :" << msg << std::endl;
              channels[i].kick_user_msg(
                  RPL_KICK(this->bot.getNickname(), channels[i].getName(),
                           clt.getNickname(), " Dont use bad words"));
              int res = channels[i].getUserIndex(clt.getNickname());
              if (res != -1)
                channels[i].remove_user(res, clt.getNickname());
            }
            return;
          }
          break;
        }
      }
      if (i == channels.size())
        send_reply(clt.getFd(),
                   ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
    } else if (line.find("\001DCC SEND ") != std::string::npos ||
               line.find("\001DCC CHAT ") != std::string::npos) {
      // handle the dcc connection and send responce to the client

      Dcc dccHandler(clt);
      dccHandler.setData(line, this->getClients());
      send_reply(dccHandler.getCltFd(), dccHandler.getClientResponce());
    } else {
      line = extract_param(command, line, 2);
      // checking for bot, and forward message to bot
      if (command[1].compare(this->bot.getNickname()) == 0) {
        this->bot.setMessage(line);
      } else {
        for (i = 0; i < clients.size(); i++) {
          if (command[1].compare(clients[i]->getNickname()) == 0) {
            send_reply(
                clients[i]->getFd(),
                RPL_PRIVMSG(clt.getNickname(), clients[i]->getNickname(), line));
            break;
          }
        }
        if (i == clients.size())
          send_reply(clt.getFd(),
                     ERR_NOSUCHNICK(clt.getNickname(), command[1]));
      }
    }
  }
}

void server::do_invite(std::vector<std::string> &command, client &clt) {
  std::stringstream ss;
  if (command.size() != 3)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  bool channel_exist = false;
  bool user_exist = false;
  int fd;
  command[2].erase(0, 1);
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getNickname().compare(command[1]) == 0) {
      user_exist = true;
      fd = clients[i]->getFd();
    } else if (this->bot.getNickname().compare(command[1]) ==
               0) // checking also for bot name
      user_exist = true;
  }
  if (!user_exist) {
    send_reply(clt.getFd(), ERR_NOSUCHNICK(clt.getNickname(), command[1]));
    return;
  }
  for (size_t i = 0; i < channels.size(); i++) {
    if (channels[i].getName().compare(command[2]) == 0 &&
        channels[i].getSize() > 0)
      channel_exist = true;
  }
  if (!channel_exist) {
    send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[2]));
    return;
  }

  for (size_t i = 0; i < channels.size(); i++) {
    if (channels[i].getName().compare(command[2]) == 0) {
      if (this->bot.getNickname().compare(command[1]) == 0 &&
          !channels[i].getIsBotJoined()) // adding the bot to the channel
      {
        std::cout << "adding bot to channel " << std::endl;
        channels[i].setIsBotJoined(true); // bot is officially joinded channel
        return;
      } else if (this->bot.getNickname().compare(command[1]) == 0 &&
                 channels[i].getIsBotJoined()) {
        std::cout << RED
                  << "The Bot already joined this channel : " << command[2]
                  << RESET << std::endl;
        return;
      }
      if (channels[i].check_nickname(command[1]))
        return send_reply(
            clt.getFd(),
            ERR_USERONCHANNEL(clt.getNickname(), command[1], command[2]));
      else if (!channels[i].check_nickname(clt.getNickname()))
        return send_reply(clt.getFd(),
                          ERR_NOTONCHANNEL(clt.getNickname(), command[2]));
      else if (channels[i].c_modes[INVITE_ONLY_M] &&
               channels[i].getOperatorIndex(clt.getNickname()) == -1)
        return send_reply(clt.getFd(),
                          ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[2]));
      send_reply(fd, RPL_INVITING(clt.getNickname(), command[1], command[2]));
    }
  }
}

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
          return send_reply(clt.getFd(),
                            ERR_NOTONCHANNEL(clt.getNickname(), command[1]));
        else if (channels[i].getOperatorIndex(clt.getNickname()) == NOT_VALID)
          return send_reply(
              clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
        int j = channels[i].getUserIndex(command[2]);
        if (j == NOT_VALID)
          return send_reply(
              clt.getFd(),
              ERR_USERNOTINCHANNEL(clt.getNickname(), command[2], command[1]));
        std::string reason;
        reason = extract_param(command, line, 3);
        if (reason.empty()) {
          reason = "Operator didn't tell why";
          channels[i].kick_user_msg(
              RPL_KICK(clt.getNickname(), command[1], command[2], reason));
        } else
          channels[i].kick_user_msg(
              RPL_KICK(clt.getNickname(), command[1], command[2], reason));
        channels[i].remove_user(j, command[2]);
        return;
      }
    }
    if (i == channels.size())
      send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  }
}

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
    return send_reply(clt.getFd(),
                      ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  if (command.size() == 2) {
    // TOPIC #channelname
    if (channels[i].getTopic().empty())
      send_reply(clt.getFd(), RPL_NOTOPIC(clt.getNickname(), command[1]));
    else
      send_reply(clt.getFd(), RPL_TOPIC(clt.getNickname(), command[1],
                                        channels[i].getTopic()));
  } else if (command.size() > 2 && command[2].at(0) == ':') {
    // TOPIC #channelname :new topic
    if (!channels[i].c_modes[TOPIC_RESTRICTION_M] ||
        (channels[i].c_modes[TOPIC_RESTRICTION_M] &&
         channels[i].getOperatorIndex(clt.getNickname()) != -1)) {
      std::string new_topic = extract_param(command, line, 2);
      channels[i].setTopic(new_topic);
      channels[i].topicToAllMembers(clt, new_topic);
    } else
      send_reply(clt.getFd(),
                 ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
  }
}

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
      if (clt_index != -1)
      {
        channels[i].addAsOperator(clt_index);
        send_reply(channels[i].user_fd(command[3]), RPL_MODE(command[3], "+o"));
      }
      return ;
    } else if (command[2].compare("-o") == 0) {
      int op_index = channels[i].getOperatorIndex(command[3]);
      if (op_index != -1)
      {
        channels[i].eraseOperator(op_index);
        send_reply(channels[i].user_fd(command[3]), RPL_MODE(command[3], "-o"));
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

void server::send_pong(std::vector<std::string> &command, client &clt) {
  if (command.size() < 2 || command[1].empty())
    send_reply(clt.getFd(),
               (ERR_NEEDMOREPARAMS(clt.getNickname(), command[0])));
  else
    send_reply(clt.getFd(),
               "PONG " + std::string(SERVER_NAME) + " " + command[1] + "\r\n");
}

void server::channel_cmds(std::string line, client *clt) {
  std::vector<std::string> command = split_line(line, " \t");
  if (command[0].compare("JOIN") == 0)
    do_join(command, clt);
  else if (command[0].compare("PRIVMSG") == 0)
    do_privmsg(command, *clt, line);
  else if (command[0].compare("TOPIC") == 0)
    do_topic(command, *clt, line);
  else if (command[0].compare("INVITE") == 0)
    do_invite(command, *clt);
  else if (command[0].compare("KICK") == 0)
    do_kick(command, *clt, line);
  else if (command[0].compare("MODE") == 0)
    do_mode(command, *clt);
  else if (command[0].compare("PING") == 0)
    send_pong(command, *clt);
  else if (command[0].compare("QUIT") == 0)
    deleteClientData(clt);
  else if (command[0].compare("NICK") != 0 && command[0].compare("USER") != 0 &&
           command[0].compare("PASS") != 0)
    send_reply(clt->getFd(), ERR_UNKNOWNCOMMAND(clt->getNickname(), command[0]));
}

void server::execute_cmds(client *clt) {
  size_t pos;
  std::string line;
  while ((pos = read_buffer[clt->getFd()].find("\r\n")) != std::string::npos) {
    line = read_buffer[clt->getFd()].substr(0, pos);
    std::cout << YELLOW << "line to parse : " << RESET;
    std::cout << line << std::endl;
    if (!line.empty()) {
      authenticate_cmds(line, *clt);
      if (clt->authentication[0] && clt->authentication[1] &&
          clt->authentication[2])
        channel_cmds(line, clt);
    }
    read_buffer[clt->getFd()].erase(0, pos + 2);
  }
}

std::string print_time_welcome() {
  std::time_t currentTime = std::time(NULL);
  std::tm *localTime = std::localtime(&currentTime);
  std::stringstream ss;

  ss << "[" << std::setw(4) << localTime->tm_year + 1900 << "-" << std::setw(2)
     << std::setfill('0') << localTime->tm_mon + 1 << "-" << std::setw(2)
     << std::setfill('0') << localTime->tm_mday << " " << std::setw(2)
     << std::setfill('0') << localTime->tm_hour << ":" << std::setw(2)
     << std::setfill('0') << localTime->tm_min << ":" << std::setw(2)
     << std::setfill('0') << localTime->tm_sec << "]" << std::endl;
  return ss.str();
}

void print_time() {
  std::time_t currentTime = std::time(NULL);
  std::tm *localTime = std::localtime(&currentTime);
  std::cout << "[" << std::setw(4) << localTime->tm_year + 1900 << "-"
            << std::setw(2) << std::setfill('0') << localTime->tm_mon + 1 << "-"
            << std::setw(2) << std::setfill('0') << localTime->tm_mday << " "
            << std::setw(2) << std::setfill('0') << localTime->tm_hour << ":"
            << std::setw(2) << std::setfill('0') << localTime->tm_min << ":"
            << std::setw(2) << std::setfill('0') << localTime->tm_sec << "]"
            << std::endl;
}

void print_ft_irc() {
  std::cout << GREEN
            << "██╗██████╗  ██████╗        ███████╗███████╗██████╗ ██╗   ██╗"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██╔══██╗██╔════╝        ██╔════╝██╔════╝██╔══██╗██║   ██║"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██████╔╝██║             ███████╗█████╗  ██████╔╝██║   ██║"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██╔══██╗██║             ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝"
            << RESET << std::endl;
  std::cout << GREEN
            << "██║██║  ██║╚██████╗███████╗███████║███████╗██║  ██║ ╚████╔╝ "
            << RESET << std::endl;
  std::cout << GREEN
            << "╚═╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝"
            << RESET << std::endl;
}

std::vector<client*> server::getClients() const { return this->clients; }
