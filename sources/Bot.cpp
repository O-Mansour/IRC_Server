#include "../includes/Bot.hpp"
#include "../includes/Bot_utils.hpp"

#include <iostream>
#include <string>
#include <vector>

Bot::Bot(std::string nickname, std::string fullname, std::string username) {
  // creating the bot
  this->fullname = fullname;
  this->username = username;
  this->nickname = nickname;
}

Bot::~Bot() {}

void Bot::handleMessage() {}

void Bot::setMessage(std::string message) {
  this->message = message;
  std::cout << "Bot_setMessage -> '" << message << "'" << std::endl;
}

std::string Bot::getResponse(std::string channelName) {
  if (this->message.empty())
    return "-1";

  if (!this->parseMessage())
    return "-1";

  std::string result;
  // check for message
  if (!this->res.empty())
    result = ":lhaj!lhaj@localhost PRIVMSG #" + channelName + " :" + this->res +
             "\r\n";

  std::cout << "BOT msg->'" << res << "'" << std::endl;
  // analyze user message and responced with command
  return result;
}

bool Bot::parseMessage() {
  std::vector<std::string> keys = splitString(this->message, ' ');
  std::string helpCmd = "!help";
  trim(keys.front());
  std::string cmd = keys.front().c_str();

  if (keys.size() == 1) // check for !
  {
    if (cmd.compare(helpCmd) == 0) {
      this->res = cmdHelp();
      return true;
    }
  }
  this->message.erase();
  return false;
}

// getters
std::string Bot::getFullname() const { return this->fullname; }
std::string Bot::getNickname() const { return this->nickname; }
std::string Bot::getUsername() const { return this->username; }
