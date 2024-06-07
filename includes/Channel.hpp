#pragma once


#include "Client.hpp"
#include <vector>
#include <unistd.h>
#include <iostream>
#include <cstring>


//bunch of colors and stuff
#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define UNDERLINE "\033[4;37m"
#define BOLD "\033\e[1m"

#include <iostream>
#include <algorithm>

# define INVITE_ONLY_M 0
# define TOPIC_RESTRICTION_M 1
# define CHANNEL_KEY_M 2
# define USER_LIMIT_M 3

class channel {
	private:
		std::string name;
		std::string topic;
		std::string key;
		size_t userLimit;
		std::vector <client> clients;
		size_t size;
		std::vector <std::string> operators;
	public:
		channel(std::string n, std::string opr);
		~channel();
		bool c_modes[4];
		std::string getName() const;
		std::string getTopic() const;
		void		setTopic(const std::string t);
		void setSize();
		size_t getSize();
		bool getCltFd(int fd);
		//cmds :
		void c_join(client &clt, std::string key);
		void c_privmsg(client &clt, std::string key);
		int kick_user(std::string &key);
		size_t		getUserLimit() const;
		void		setUserLimit(const size_t limit);
		std::string getKey() const;
		void		setKey(const std::string t);
		void		addAsOperator(std::string nick);
		void		eraseOperator(std::string nick);
		bool		isOperator(std::string nick);
};
