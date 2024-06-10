#include "../includes/Channel.hpp"
#include <sstream>
#include <string>

channel::channel(std::string n, std::string opr) : name(n), userLimit(0) {
  this->isBotJoined = false;
  for (int i = 0; i < 4; i++)
    c_modes[i] = false;
  operators.push_back(opr);
}

channel::~channel() {}

std::string channel::getName() const { return name; }

std::string channel::getTopic() const { return topic; }

void channel::setTopic(const std::string t) { topic = t; }

size_t channel::getSize() { return clients.size(); }

size_t channel::getUserLimit() const { return userLimit; }

void channel::setUserLimit(const size_t limit) { userLimit = limit; }

std::string channel::getKey() const { return key; }

void channel::setKey(const std::string k) { this->key = k; }

void channel::addAsOperator(std::string nick) {
  // check if the client is in the channel
  size_t i;
  for (i = 0; i < clients.size(); i++) {
    if (!clients[i].getNickname().compare(nick)) {
      operators.push_back(nick);
      break;
    }
  }
  if (i == clients.size())
    std::cout << "client must join the channel" << std::endl;
}

void channel::eraseOperator(std::string nick) {
  if (isOperator(nick)) {
    std::vector<std::string>::iterator it;
    it = std::find(operators.begin(), operators.end(), nick);
    operators.erase(it);
  } else
    std::cout << "client is not an operator" << std::endl;
}

bool channel::isOperator(std::string nick) const {
  std::vector<std::string>::const_iterator it;
  it = std::find(operators.begin(), operators.end(), nick);
  if (it != operators.end())
    return true;
  return false;
}

void channel::c_join(client &clt, std::string k) {
  if (key.empty() || !this->key.compare(k)) {
    size_t i;
    for (i = 0; i < clients.size(); i++) {
      if (clt.getFd() == clients[i].getFd())
        break;
    }
    if (i == clients.size())
    {
      clients.push_back(clt);
      send_reply(clt.getFd(), RPL_JOIN(clt.getNickname(), this->name));
      if (!this->topic.empty())
        send_reply(clt.getFd(), RPL_TOPIC(clt.getNickname(), this->name, this->topic));
      std::string clients_list = getClientsList();
      send_reply(clt.getFd(), RPL_NAMREPLY(clt.getNickname(), this->name, clients_list));
      send_reply(clt.getFd(), RPL_ENDOFNAMES(clt.getNickname(), this->name));
    }
  }
  else
    send_reply(clt.getFd(), ERR_BADCHANNELKEY(clt.getNickname(), this->name));
}

bool channel::getCltFd(int fd) {
  for (size_t i = 0; i < clients.size(); i++)
    if (clients[i].getFd() == fd)
      return true;
  return false;
}

void channel::c_privmsg(client &clt, std::string key) {
  for (size_t i = 0; i < clients.size(); i++) {
    std::stringstream ss;
    ss << UNDERLINE << "New message from " << clt.getNickname() << " :\r\n"
       << RESET << std::endl;
    write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
    write(clients[i].getFd(), "#", 1);
    write(clients[i].getFd(), this->getName().c_str(), this->getName().size());
    write(clients[i].getFd(), " : ", 3);
    write(clients[i].getFd(), key.c_str(), key.size());
    write(clients[i].getFd(), "\n", 1);
  }
}

void channel::msgToAllMemebers(std::string nickname, std::string key) {
  for (size_t i = 0; i < clients.size(); i++) {
    std::stringstream ss;
    ss << UNDERLINE << "New message from " << nickname << " :\r\n"
       << RESET << std::endl;
    write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
    write(clients[i].getFd(), "#", 1);
    write(clients[i].getFd(), this->getName().c_str(), this->getName().size());
    write(clients[i].getFd(), " : ", 3);
    write(clients[i].getFd(), key.c_str(), key.size());
    write(clients[i].getFd(), "\n", 1);
  }
}

int channel::getUserIndex(const std::string &nick) {
  for (size_t i = 0; i < clients.size(); i++) {
    if (!clients[i].getNickname().compare(nick))
      return i;
  }
  return -1;
}

void channel::remove_user(int index, const std::string &nick) {
  clients.erase(clients.begin() + index);
  std::vector<std::string>::iterator it;
  it = std::find(operators.begin(), operators.end(), nick);
  if (it != operators.end())
    operators.erase(it);
}

int channel::user_fd(std::string &key) {
  for (size_t i = 0; i < clients.size(); i++) {
    if (!clients[i].getNickname().compare(key))
      return clients[i].getFd();
  }
  return -1;
}

std::string channel::getClientsList() const
{
  std::string res;
  for (size_t i = 0; i < clients.size(); i++)
  {
    if (i != 0)
      res += " ";
    if (isOperator(clients[i].getNickname()))
      res += "@";
    res += clients[i].getNickname();
  }
  return res;
}


// getters and setter for bot
bool channel::getIsBotJoined() const { return this->isBotJoined; }

void channel::setIsBotJoined(bool val) { this->isBotJoined = val; }
