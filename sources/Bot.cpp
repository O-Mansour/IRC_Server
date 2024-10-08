#include "Bot.hpp"
#include "Bot_utils.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Bot::Bot(std::string nickname, std::string fullname, std::string username) {
  // creating the bot
  this->badWords = extractData();

  this->fullname = fullname;
  this->username = username;
  this->nickname = nickname;
}

Bot::~Bot() {}

void Bot::handleMessage() {}

void Bot::setMessage(std::string message) {
  this->message = message;
}

std::string Bot::getResponse(std::string channelName) {
  std::string result;

  if (this->message.empty())
    return "-1";
  // check for bad word

  if (!this->parseMessage())
    return "-1";

  // check for message
  if (!this->res.empty())
    result = ":lhaj!lhaj@localhost PRIVMSG #" + channelName + " :" + this->res +
             "\r\n";

  // analyze user message and responced with command
  return result;
}

bool Bot::parseMessage() {
  std::vector<std::string> keys = splitString(this->message, ' ');
  std::string helpCmd = "!help";
  std::string cmd = keys.front().c_str();

  if (this->barWordChecker()) {
    this->res = this->badRes;
    return true;
  }

  if (keys.size() == 1) // check for !
  {
    if (cmd.compare(helpCmd) == 0) {
      this->res = cmdHelp();
      return true;
    } else if (cmd.compare("!quote") == 0) {
      this->res = cmdQuote();
      return true;
    }
  }
  this->message.erase();
  return false;
}

bool Bot::barWordChecker() {
  std::vector<std::string> tmp = splitString(this->message, ' ');
  std::stringstream ss;

  if (tmp.empty())
    return false;

  vec_it it;
  for (it = tmp.begin(); it != tmp.end(); ++it) {
    if (std::find(this->badWords.begin(), this->badWords.end(), *it) !=
        this->badWords.end()) {
      this->badRes = "-2";
      return true;
    }
  }

  return false;
}

// getters
std::string Bot::getFullname() const { return this->fullname; }
std::string Bot::getNickname() const { return this->nickname; }
std::string Bot::getUsername() const { return this->username; }
