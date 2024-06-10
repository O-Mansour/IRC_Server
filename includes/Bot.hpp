#pragma once

#include <string>
#include <vector>

/* lhaj:
 * Lhaj is an Irc bot that can be invited to a channel
 * and act like a moderator if role is given.
 * and reply with time for !time, and !quote
 */

typedef std::vector<std::string>::iterator vec_it;

class Bot {
private:
  std::vector<std::string> badWords;
  std::string fullname;
  std::string username;
  std::string nickname;
  std::string message;
  std::string res;
  std::string badRes;
  std::vector<std::string> msgWords;

public:
  Bot(std::string nickname, std::string username, std::string fullname);
  ~Bot();

  void handleMessage();
  void setMessage(std::string message);
  std::string getResponse(std::string channelName);
  bool parseMessage();
  bool barWordChecker();

  // getters
  std::string getNickname() const;
  std::string getFullname() const;
  std::string getUsername() const;
};
