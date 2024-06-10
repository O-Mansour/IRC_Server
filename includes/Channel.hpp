#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

// bunch of colors and stuff
#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define UNDERLINE "\033[4;37m"
#define BOLD "\033\e[1m"

#include <algorithm>
#include <iostream>

#define INVITE_ONLY_M 0
#define TOPIC_RESTRICTION_M 1
#define CHANNEL_KEY_M 2
#define USER_LIMIT_M 3

class channel {
private:
  std::string name;
  std::string topic;
  std::string key;
  size_t userLimit;
  size_t size;
  std::vector<client> clients;
  std::vector<std::string> operators; // nickname may not works

  bool isBotJoined;

public:
  channel(std::string n, std::string opr);
  ~channel();
  bool c_modes[4];
  std::string getName() const;
  std::string getTopic() const;
  void setTopic(const std::string t);
  size_t getSize();
  bool getCltFd(int fd);
  void c_join(client &clt, std::string key);
  void c_privmsg(client &clt, std::string key);
  int getUserIndex(const std::string &nick);
  void msgToAllMemebers(std::string key);
  void remove_user(int index, const std::string &nick);
  int user_fd(std::string &key);
  size_t getUserLimit() const;
  void setUserLimit(const size_t limit);
  std::string getKey() const;
  void setKey(const std::string t);
  void addAsOperator(std::string nick);
  void eraseOperator(std::string nick);
	bool check_nickname(std::string str);
  bool isOperator(std::string nick) const;
  std::string getClientsList() const;

  // setters and getter for bot
  void setIsBotJoined(bool val);
  bool getIsBotJoined() const;
};
