#include "Channel.hpp"
#include "replies.hpp"

channel::channel(std::string n, client *opr) : name(n), userLimit(0) {
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

void channel::eraseOperator(int op_index) {
  operators.erase(operators.begin() + op_index);
}

void channel::addAsOperator(int clt_index) {
  // check if he is not an operator already
  if (getOperatorIndex(clients[clt_index]->getNickname()) == NOT_VALID)
    operators.push_back(clients[clt_index]);
}

int channel::getOperatorIndex(const std::string &nick) const {
  for (size_t i = 0; i < operators.size(); i++) {
    if (operators[i]->getNickname().compare(nick) == 0)
      return i;
  }
  return -1;
}

void channel::c_join(client *clt, std::string k) {
  if (this->key.empty() || this->key.compare(k) == 0) {
    size_t i;
    for (i = 0; i < clients.size(); i++) {
      if (clt->getFd() == clients[i]->getFd())
        break;
    }
    if (i == clients.size()) {
      clients.push_back(clt);
      send_reply(clt->getFd(), RPL_JOIN(clt->getNickname(), this->name));
      if (!this->topic.empty())
        send_reply(clt->getFd(), RPL_TOPIC(clt->getNickname(), this->name, this->topic));
      std::string clients_list = getClientsList();
      send_reply(clt->getFd(), RPL_NAMREPLY(clt->getNickname(), this->name, clients_list));
      send_reply(clt->getFd(), RPL_ENDOFNAMES(clt->getNickname(), this->name));
    }
  } else
    send_reply(clt->getFd(), ERR_BADCHANNELKEY(clt->getNickname(), this->name));
}

bool channel::getCltFd(int fd) {
  for (size_t i = 0; i < clients.size(); i++)
    if (clients[i]->getFd() == fd)
      return true;
  return false;
}

bool channel::check_nickname(std::string str) {
  for (size_t i = 0; i < clients.size(); i++)
    if (str.compare(clients[i]->getNickname()) == 0)
      return true;
  return false;
}

void channel::c_privmsg(client &clt, std::string msg) {
  for (size_t i = 0; i < clients.size(); i++) {
    if (clt.getFd() != clients[i]->getFd())
      send_reply(clients[i]->getFd(), RPL_PRIVMSG(clt.getNickname(), "#" + this->getName(), msg));
  }
}

void channel::msgToAllMemebers(std::string msg) {
  for (size_t i = 0; i < clients.size(); i++)
    send_reply(clients[i]->getFd(), msg);
}

int channel::getUserIndex(const std::string &nick) {
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getNickname().compare(nick) == 0)
      return i;
  }
  return -1;
}

void channel::remove_user(int index, const std::string &nick) {
  clients.erase(clients.begin() + index);
  if (getOperatorIndex(nick) != NOT_VALID)
    eraseOperator(getOperatorIndex(nick));
}

int channel::user_fd(std::string &nick) {
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getNickname().compare(nick) == 0)
      return clients[i]->getFd();
  }
  return -1;
}

std::string channel::getClientsList() const {
  std::string res;
  for (size_t i = 0; i < clients.size(); i++) {
    if (i != 0)
      res += " ";
    if (getOperatorIndex(clients[i]->getNickname()) != -1)
      res += "@";
    res += clients[i]->getNickname();
  }
  return res;
}

void channel::addToInvitedFds(int clt_fd) {
  if (getInviteIndex(clt_fd) == NOT_VALID)
    invited_fds.push_back(clt_fd);
}

void channel::removeFromInvitedFds(int index) {
  invited_fds.erase(invited_fds.begin() + index);
}

int channel::getInviteIndex(int clt_fd)
{
  for (size_t i = 0; i < invited_fds.size(); i++) {
    if (invited_fds[i] == clt_fd)
      return i;
  }
  return NOT_VALID;
}

// getters and setter for bot
bool channel::getIsBotJoined() const { return this->isBotJoined; }

void channel::setIsBotJoined(bool val) { this->isBotJoined = val; }
