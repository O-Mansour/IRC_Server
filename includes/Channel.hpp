#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Client.hpp"
#include <vector>
#include <iostream>

class channel {
	private:
		std::string name;
		std::string topic;
		std::string key;
		std::vector <client> clients;
	public:
		channel(std::string n);
		~channel();
		std::string getName() const;
		std::string getTopic() const;
		void		setTopic(const std::string t);

		//cmds :
		void c_join(client &clt, std::string k);
};

#endif
