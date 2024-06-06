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

class channel {
	private:
		std::string name;
		std::string topic;
		std::string key;
		std::vector <client> clients;
		size_t size;
	public:
		channel(std::string n);
		~channel();
		std::string getName() const;
		std::string getTopic() const;
		void		setTopic(const std::string t);
		void setSize();
		size_t getSize();
		bool getCltFd(int fd);
		//cmds :
		void c_join(client &clt, std::string key);
		void c_privmsg(client &clt, std::string key);
};
