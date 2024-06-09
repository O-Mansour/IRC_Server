#include "Bot.hpp"

#include <iostream>

Bot::Bot(std::string nickname, std::string fullname, std::string username)
{
	// creating the bot
	this->fullname = fullname;
	this->username = username;
	this->nickname = nickname;
}

Bot::~Bot() {}

void Bot::handleMessage() {}

void Bot::setMessage(std::string message)
{
	this->message = message;
	std::cout << "Bot::setMessage -> '" << message << "'" << std::endl;
}

std::string Bot::getResponse()
{
	// check for message 
	if (this->message.empty())
		return NULL;
	// analyze user message and responced with command
	this->message.erase();
	return "lhaj will answer you, please wait";
}

// getters
std::string Bot::getFullname() const 
{
	return  this->fullname;
}

std::string Bot::getNickname() const
{
	return  this->nickname;
}

std::string Bot::getUsername() const
{
	return  this->username;
}


