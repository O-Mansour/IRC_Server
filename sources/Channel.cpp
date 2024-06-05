#include "../includes/Channel.hpp"

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
