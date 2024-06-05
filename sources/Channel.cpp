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

std::string channel::getTopic() const
{
	return topic;
}

void channel::setTopic(const std::string t)
{
	topic = t;
}

void channel::c_join(client &clt, std::string k)
{
	if (key.empty() || !this->key.compare(k))
	{
		size_t i;
		for (i = 0; i < clients.size(); i++)
		{
			if (clt.getFd() == clients[i].getFd())
			{
				std::cout << "Client has joined the channel already" << std::endl;
				break ;
			}
		}
		if (i == clients.size())
			clients.push_back(clt);
	}
	else
		std::cout << "Channel key incorrect" << std::endl;
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
