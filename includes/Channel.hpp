#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Client.hpp"
#include <vector>

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
		// void		setName(const std::string n);

		//cmds :
		void c_join(client &clt, std::string key);
};

#endif
