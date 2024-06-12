#include "../includes/Server.hpp"
#include "../includes/Bot.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

server::server(int p, std::string pass)
    : port(p), password(pass), bot("lhaj", "lhaj Molshi", "lhaj") {
  // creating a simple tcp server
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
  // setup for poll()
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
  client clt(nb);
  clients.push_back(clt);
  print_time();
  std::cout << YELLOW << "New client connected with fd : " << nb << RESET
            << std::endl;
}

void server::deleteClientData(client &clt) {
  // erase him from poll_fds vector
  for (size_t i = 0; i < poll_fds.size(); i++) {
    if (poll_fds[i].fd == clt.getFd())
      poll_fds.erase(poll_fds.begin() + i);
  }
  // erase him from clients vector
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i].getFd() == clt.getFd())
      clients.erase(clients.begin() + i);
  }
  // erase his read_buffer
  std::map<int, std::string>::iterator it = read_buffer.find(clt.getFd());
  if (it != read_buffer.end())
    read_buffer.erase(it);
  // remove him from all channels
  for (size_t i = 0; i < channels.size(); i++) {
    int uIndex = channels[i].getUserIndex(clt.getNickname());
    if (uIndex != NOT_VALID)
      channels[i].remove_user(uIndex, clt.getNickname());
  }
  // close his fd
  close(clt.getFd());
}

void  send_reply(int fd, std::string str) {
  send(fd, str.c_str(), str.length(), 0);
}

void server::HandleData(client &clt) {
  char buf[BUFFER_SIZE];
  ssize_t rn = recv(clt.getFd(), buf, BUFFER_SIZE - 1, 0);
  if (rn < 0)
    throw std::runtime_error("An error occurred with recv()");
  else if (rn == 0) {
    print_time();
    std::cout << RED << "Client with fd : " << clt.getFd() << " disconnected"
              << RESET << std::endl;
    deleteClientData(clt);
  } else {
    buf[rn] = '\0';
    read_buffer[clt.getFd()].append(buf);
    execute_cmds(clt);
  }
}

void server::start() {
  while (true) {
    if (poll(&poll_fds[0], poll_fds.size(), -1) == -1)
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

std::vector<std::string> split_line(std::string line) {
  std::vector<std::string> res;
  char *buff = new char[line.size() + 1];
  char *word;

  buff = std::strcpy(buff, line.c_str());
  word = std::strtok(buff, " 	\r");
  while (word) {
    res.push_back(word);
    word = std::strtok(NULL, " 	\r");
  }
  delete[] buff;
  return res;
}

void server::check_password(std::vector<std::string> &command, client &clt) {
  std::string reply;
  if (command.size() < 2)
    send_reply(clt.getFd(), (ERR_NEEDMOREPARAMS()));
  else {
    if (clt.authentication[0])
      send_reply(clt.getFd(), (ERR_ALREADYREGISTERED(clt.getNickname())));
    else {
      if (!command[1].compare(password))
        clt.authentication[0] = true;
      else
        send_reply(clt.getFd(), (ERR_PASSWDMISMATCH()));
    }
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
    send_reply(clt.getFd(), ERR_ERRONEUSNICKNAME());
  else if (!command[1].compare(this->bot.getNickname())) {
    send_reply(clt.getFd(), ERR_NICKNAMEINVALID(command[1]));
  } else {
    size_t i;
    for (i = 0; i < clients.size(); i++) {
      if (!clients[i].getNickname().compare(command[1]) &&
          !clients[i].authentication[1]) {
        send_reply(clt.getFd(),
                   ERR_NICKNAMEINUSE(clt.getNickname(), command[1]));
        break;
      }
    }
    if (i == clients.size()) {
      if (clt.authentication[1]) {
        send_reply(clt.getFd(),
                   ERR_NICKNAMECHANGE(clt.getNickname(), command[1]));
        clt.setNickname(command[1]);
      } else {
        clt.setNickname(command[1]);
        clt.authentication[1] = true;
      }
    }
  }
}

void server::register_user(client &clt) {
  send_reply(clt.getFd(), RPL_WELCOME(clt.getNickname()));
  send_reply(clt.getFd(), RPL_YOURHOST());
  send_reply(clt.getFd(), RPL_CREATED());
  send_reply(clt.getFd(), RPL_MYINFO());
  send_reply(clt.getFd(), RPL_ISUPPORT());
}

void server::check_username(std::vector<std::string> &command, client &clt,
                            std::string &line) {
  if (command.size() < 5 )
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  else if (clt.authentication[2])
    return send_reply(clt.getFd(), ERR_ALREADYREGISTERED(clt.getNickname()));
  else if (command[2].compare("0") || command[3].compare("*"))
    return send_reply(clt.getFd(), ERR_USERFORMAT());
  else if (command[4].at(0) != ':')
    return send_reply(clt.getFd(), ERR_USERSYNTAX());
  else {
    if (command[4].length() < 2)
      return send_reply(clt.getFd(), ERR_USERSYNTAX());
    clt.setUsername(command[1]);
    line = extract_param(command, line, 4);
    clt.setFullname(line);
    clt.authentication[2] = true;
    register_user(clt);
  }
}

void server::authenticate_cmds(std::string line, client &clt) {
  std::string reply;
  std::vector<std::string> command = split_line(line);
  if (!command[0].compare("PASS"))
    check_password(command, clt);
  if (!command[0].compare("NICK") && clt.authentication[0])
    check_nickname(command, clt);
  else if (!command[0].compare("NICK"))
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  if (!command[0].compare("USER") && clt.authentication[0] &&
      clt.authentication[1])
    check_username(command, clt, line);
  else if (!command[0].compare("USER"))
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
}

std::vector<std::string> tmp_split(std::string line, std::string delim) {
  std::vector<std::string> res;
  char *buff = new char[line.size() + 1];
  char *word;

  buff = std::strcpy(buff, line.c_str());
  word = std::strtok(buff, delim.c_str());
  while (word) {
    res.push_back(word);
    word = std::strtok(NULL, delim.c_str());
  }
  delete[] buff;
  return res;
}

void server::do_join(std::vector<std::string> &command, client &clt)
{
  if (command.size() < 2)
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  // "JOIN 0" to leave all channels
  if (!command[1].compare("0"))
  {
    for (size_t i = 0; i < channels.size(); i++) {
      int uIndex = channels[i].getUserIndex(clt.getNickname());
      if (uIndex != NOT_VALID)
        channels[i].remove_user(uIndex, clt.getNickname());
    }
    return ;
  }
  std::vector<std::string> chan_list = tmp_split(command[1], ",");
  std::vector<std::string> keys_list;
  if (command.size() > 2)
    keys_list = tmp_split(command[2], ",");
  for (size_t i = 0; i < chan_list.size(); i++) {
    if (chan_list[i].at(0) != '#')
    {
      send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
      continue ;
    }
    chan_list[i].erase(0, 1);
    if (chan_list[i].length() == 0)
    {
      send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
      continue ;
    }
    std::string c_key = "";
    if (keys_list.size() > i)
      c_key = keys_list[i];
    size_t j;
    for (j = 0; j < channels.size(); j++) {
      if (!channels[j].getName().compare(chan_list[i])) {
        // check channel modes
        if (channels[j].c_modes[INVITE_ONLY_M])
          send_reply(clt.getFd(), ERR_INVITEONLYCHAN(clt.getNickname(), chan_list[i]));
        else if (channels[j].c_modes[USER_LIMIT_M] && channels[j].getSize() == channels[j].getUserLimit())
          send_reply(clt.getFd(), ERR_CHANNELISFULL(clt.getNickname(), chan_list[i]));
        else
          channels[j].c_join(clt, c_key);
        break;
      }
    }
    if (j == channels.size()) {
      // create channel and add the user to it
      channel chnl(chan_list[i], clt.getNickname());
      chnl.c_join(clt, "");
      channels.push_back(chnl);
    }
  }
}

void server::do_privmsg(std::vector<std::string> &command, client &clt,
                        std::string line) {
  if (command.size() < 3)
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  else if (command[2].at(0) != ':')
    send_reply(clt.getFd(), ERR_MSGSYNTAX());
  else {
    size_t i;
    if (command[1].at(0) == '#') {
      command[1].erase(0, 1);
      line = extract_param(command, line, 2);
      for (i = 0; i < channels.size(); i++) {
        if (!channels[i].getName().compare(command[1])) {
          channels[i].c_privmsg(clt, line);
          // TODO: if the bot joined channel. check forward messages to bot
          if (channels[i].getIsBotJoined()) {
            this->bot.setMessage(line);
            std::string msg = this->bot.getResponse(channels[i].getName());
            if (msg != "-1")
              channels[i].msgToAllMemebers(msg);
            return;
          }
        }
        if (i == clients.size())
          send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
      }
    } else {
      line = extract_param(command, line, 2);
      // checking for bot, and forward message to bot
      if (!command[1].compare(this->bot.getNickname())) {
        this->bot.setMessage(line);
      } else {
        std::string msg_str =
            ":" + clt.getNickname() + "!~h@localhost PRIVMSG ";
        for (i = 0; i < clients.size(); i++) {
          if (!command[1].compare(clients[i].getNickname())) {
            std::string target = clients[i].getNickname();
            msg_str += target + " :" + line + "\n";
            write(clients[i].getFd(), msg_str.c_str(), msg_str.length());
            break;
          }
        }
        if (i == clients.size())
          send_reply(clt.getFd(), ERR_NOSUCHNICK());
      }
    }
  }
}

void server::do_invite(std::vector<std::string> &command, client &clt) {
  std::stringstream ss;
  if (command.size() != 3)
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  else {
    bool channel_exist = false;
    bool clt_part_in_it = false;
    bool user_exist = false;
    int fd;
    command[2].erase(0, 1);
    for (size_t i = 0; i < clients.size(); i++) {
      if (!clients[i].getNickname().compare(command[1])) {
        user_exist = true;
        fd = clients[i].getFd();
      }
      // add your own bool here
      // } else if (!this->bot.getNickname().compare(
      //                command[1])) // checking also for bot name
      // {
      //   user_exist = true;
      // }
    }
    if (!user_exist) {
      send_reply(clt.getFd(), ERR_NOSUCHNICK());
      return;
    }
    for (size_t i = 0; i < channels.size(); i++) {
      if (!channels[i].getName().compare(command[2]) &&
          channels[i].getSize() > 0)
        channel_exist = true;
    }
    if (!channel_exist) {
      send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
      return;
    }
    for (size_t i = 0; i < channels.size(); i++) {
      // if (!this->bot.getNickname().compare(command[1]) &&
      //     !channels[i].getIsBotJoined()) // adding the bot to the channel
      // {
      //   std::cout << "adding bot to channel " << std::endl;
      //   channels[i].setIsBotJoined(true);
      // } else if (!this->bot.getNickname().compare(command[1]) &&
      //            channels[i].getIsBotJoined()) {
      //   std::cout << RED
      //             << "The Bot already joined this channel : " << command[2]
      //             << RESET << std::endl;
      //   write(fd, ss.str().c_str(), ss.str().size());
      //   return;
      // }
      if (channels[i].check_nickname(command[1])) {
        send_reply(clt.getFd(),ERR_USERONCHANNEL(command[1], command[2]));
        return ;
      }else if (channels[i].c_modes[INVITE_ONLY_M] && !channels[i].isOperator(clt.getNickname())) {
				send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(command[1], command[2]));
        return;
      }else if (channels[i].check_nickname(clt.getNickname()))
				clt_part_in_it = true;
    }
    if (clt_part_in_it) {
			send_reply(fd, RPL_INVITING(clt.getNickname(), command[1], command[2]));
    } else
				send_reply(clt.getFd(), ERR_NOTONCHANNEL(clt.getNickname(), command[2]));
  }
}

void server::do_kick(std::vector<std::string> &command, client &clt) {
  std::stringstream ss;
  if (command.size() != 3 && command.size() != 4)
    std::cout << "Not valid args" << std::endl;
  else {
    command[1].erase(0, 1);
    size_t i = 0;
    for (; i < channels.size(); i++) {
      if (!channels[i].getName().compare(command[1])) {
        if (!channels[i].isOperator(clt.getNickname())) {
          std::cout << "You are not an operator" << std::endl;
          return;
        }
        int j = channels[i].getUserIndex(command[2]);
        if (j == -1) {
          std::cout << RED << "This User doesn't exist in this channel" << RESET
                    << std::endl;
          return;
        }
        ss << RED << clt.getNickname() << " Kicked your ass from the channel"
           << RESET << std::endl;
        ;
        if (command.size() == 4)
          ss << UNDERLINE << "Reason: " << RESET << command[3] << std::endl;
        write(channels[i].user_fd(command[2]), ss.str().c_str(),
              ss.str().size());
        channels[i].remove_user(j, command[2]);
        return;
      }
    }
    std::cout << RED << "This channel doesn't exist" << RESET << std::endl;
  }
}

void server::do_topic(std::vector<std::string> &command, client &clt,
                      std::string line) {
  if (command.size() < 2 || command[1].at(0) != '#')
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  command[1].erase(0, 1);
  size_t i;
  for (i = 0; i < channels.size(); i++) {
    if (!channels[i].getName().compare(command[1]))
      break ;
  }
  if (i == channels.size())
    return send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  if (command.size() == 2)
  {
    // TOPIC #channelname
    if (channels[i].getTopic().empty())
      send_reply(clt.getFd(), RPL_NOTOPIC(clt.getNickname(), command[1]));
    else
      send_reply(clt.getFd(), RPL_TOPIC(clt.getNickname(), command[1], channels[i].getTopic()));
  }
  else if (command.size() > 2 && command[2].at(0) == ':') {
    // TOPIC #channelname :new topic
    if (!channels[i].c_modes[TOPIC_RESTRICTION_M]
        || (channels[i].c_modes[TOPIC_RESTRICTION_M] && channels[i].isOperator(clt.getNickname())))
    {
      std::string new_topic = extract_param(command, line, 2);
      channels[i].setTopic(new_topic);
      channels[i].topicToAllMembers(clt, new_topic);
    }
    else
      send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
  }
}

long isValidInteger(const std::string &str)
{
  if (str.empty() || str.length() > 10)
    return 0;
  for (size_t i = 0; i < str.length(); i++)
  {
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
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  // user's modes aren't supported
  else if (command[1].at(0) != '#')
    return send_reply(clt.getFd(), ERR_UMODEUNKNOWNFLAG(clt.getNickname()));
  command[1].erase(0, 1);
  if (command[1].empty())
    return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
  // get the channel index
  size_t i;
  for (i = 0; i < channels.size(); i++) {
    if (!channels[i].getName().compare(command[1]))
      break ;
  }
  if (i == channels.size())
    return send_reply(clt.getFd(), ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
  if (command.size() == 2)
  {
    std::string modes;
    if (channels[i].c_modes[INVITE_ONLY_M])
      modes += "i";
    if (channels[i].c_modes[TOPIC_RESTRICTION_M])
      modes += "t";
    if (channels[i].c_modes[CHANNEL_KEY_M])
      modes += "k";
    if (channels[i].c_modes[USER_LIMIT_M])
      modes += "l";
    return send_reply(clt.getFd(), RPL_CHANNELMODEIS(clt.getNickname(), command[1], modes));
  }
  if (!channels[i].isOperator(clt.getNickname()))
    return send_reply(clt.getFd(), ERR_CHANOPRIVSNEEDED(clt.getNickname(), command[1]));
  if (command.size() == 3) {
    // cases : +i -i +t -t -k -l
    if (!command[2].compare("+i"))
      channels[i].c_modes[INVITE_ONLY_M] = true;
    else if (!command[2].compare("-i"))
      channels[i].c_modes[INVITE_ONLY_M] = false;
    else if (!command[2].compare("+t"))
      channels[i].c_modes[TOPIC_RESTRICTION_M] = true;
    else if (!command[2].compare("-t"))
      channels[i].c_modes[TOPIC_RESTRICTION_M] = false;
    else if (!command[2].compare("-k")) {
      channels[i].setKey("");
      channels[i].c_modes[CHANNEL_KEY_M] = false;
    } else if (!command[2].compare("-l")) {
      channels[i].setUserLimit(0);
      channels[i].c_modes[USER_LIMIT_M] = false;
    }
  }
  else if (command.size() == 4) {
    // cases : +k +l +o -o
    if (!command[2].compare("+k")) {
      channels[i].setKey(command[3]);
      channels[i].c_modes[CHANNEL_KEY_M] = true;
    } else if (!command[2].compare("+l")) {
      // checking is the number valid
      long num = isValidInteger(command[3]);
      if (num != 0)
      {
        channels[i].setUserLimit(std::atoi(command[3].c_str()));
        channels[i].c_modes[USER_LIMIT_M] = true;
      }
    } else if (!command[2].compare("+o") && channels[i].check_nickname(command[3]))
      channels[i].addAsOperator(command[3]);
    else if (!command[2].compare("-o")  && channels[i].check_nickname(command[3]))
      channels[i].eraseOperator(command[3]);
  }
}

void server::send_pong(std::vector<std::string> &command, client &clt) {
  if (command.size() < 2 || command[1].empty())
    send_reply(clt.getFd(), (ERR_NEEDMOREPARAMS()));
  else
    send_reply(clt.getFd(), "PONG " + std::string(SERVER_NAME) + " " + command[1] + "\r\n");
}

void server::channel_cmds(std::string line, client &clt) {
  std::vector<std::string> command = split_line(line);
  if (!command[0].compare("JOIN"))
    do_join(command, clt);
  else if (!command[0].compare("PRIVMSG"))
    do_privmsg(command, clt, line);
  else if (!command[0].compare("TOPIC"))
    do_topic(command, clt, line);
  else if (!command[0].compare("INVITE"))
    do_invite(command, clt);
  else if (!command[0].compare("KICK"))
    do_kick(command, clt);
  else if (!command[0].compare("MODE"))
    do_mode(command, clt);
  else if (!command[0].compare("PING"))
    send_pong(command, clt);
  else
    send_reply(clt.getFd(), ERR_UNKNOWNCOMMAND(clt.getNickname(), command[0]));
}

void server::execute_cmds(client &clt) {
  size_t pos;
  std::string line;
  // may need to implement \r\n as end of cmd
  while ((pos = read_buffer[clt.getFd()].find("\n")) != std::string::npos) {
    line = read_buffer[clt.getFd()].substr(0, pos);
    std::cout << YELLOW << "line to parse : " << RESET;
    std::cout << line << std::endl;
    if (!line.empty() && line.compare("\r") != 0) {
      authenticate_cmds(line, clt);
      if (clt.authentication[0] && clt.authentication[1] &&
          clt.authentication[2])
        channel_cmds(line, clt);
    }
    read_buffer[clt.getFd()].erase(0, pos + 1);
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
  std::cout << GREEN << "███████╗████████╗     ██╗██████╗  ██████╗" << RESET
            << std::endl;
  std::cout << GREEN << "██╔════╝╚══██╔══╝     ██║██╔══██╗██╔════╝" << RESET
            << std::endl;
  std::cout << GREEN << "█████╗     ██║        ██║██████╔╝██║     " << RESET
            << std::endl;
  std::cout << GREEN << "██╔══╝     ██║        ██║██╔══██╗██║     " << RESET
            << std::endl;
  std::cout << GREEN << "██║        ██║███████╗██║██║  ██║╚██████╗" << RESET
            << std::endl;
  std::cout << GREEN << "╚═╝        ╚═╝╚══════╝╚═╝╚═╝  ╚═╝ ╚═════╝" << RESET
            << std::endl;
  std::cout << GREEN << "                                         " << RESET
            << std::endl;
}
