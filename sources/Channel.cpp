#include "../includes/Channel.hpp"
#include <sstream>

channel::channel(std::string n) : name(n)
{
}

channel::~channel()
{
}

std::string channel::getName() const
{
	return name;
}

void channel::c_join(client &clt, std::string key)
{
	(void) clt;
	(void) key;

	// to be continued
}

void channel::c_privmsg(client &clt, std::string key){
	for (size_t i = 0; i < clients.size(); i++){
		std::stringstream ss;
		ss << UNDERLINE << "New message from " << clt.getNickname() << " :" << RESET << std::endl;
		write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
		write(clients[i].getFd(), "#", 1);
		write(clients[i].getFd(), this->getName().c_str(), this->getName().size());
		write(clients[i].getFd(), key.c_str(), key.size());
		write(clients[i].getFd(), "\n", 1);
	}
}
