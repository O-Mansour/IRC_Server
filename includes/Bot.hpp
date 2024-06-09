#pragma once

#include <string>

/* lhaj:
 * Lhaj is an Irc bot that can be invited to a channel
 * and act like a moderator if role is given.
 * and reply with time for !time, and !quote
 */

class Bot {
private:
	std::string fullname;
	std::string username;
	std::string nickname;

	std::string message;

public:
	Bot(std::string nickname, std::string username, std::string fullname);
	~Bot();

	void handleMessage();
	void setMessage(std::string message);
	std::string getResponse();

	// getters
	std::string getNickname() const;
	std::string getFullname() const;
	std::string getUsername() const;
};
