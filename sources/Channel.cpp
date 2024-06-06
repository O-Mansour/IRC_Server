#include "../includes/Channel.hpp"

channel::channel(std::string n, std::string opr) : name(n), userLimit(0)
{
	for (int i = 0; i < 4; i++)
		c_modes[i] = false;
	operators.push_back(opr);
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

size_t	channel::getUserLimit() const
{
	return userLimit;
}

void	channel::setUserLimit(const size_t limit)
{
	userLimit = limit;
}

std::string	channel::getKey() const
{
	return key;
}

void	channel::setKey(const std::string k)
{
	this->key = k;
}

void	channel::addAsOperator(std::string nick)
{
	// check if the client is in the channel
	size_t i;
	for (i = 0; i < clients.size(); i++)
	{
		if (!clients[i].getNickname().compare(nick))
		{
			operators.push_back(nick);
			break ;
		}
	}
	if (i == clients.size())
		std::cout << "client must join the channel" << std::endl;
}

void	channel::eraseOperator(std::string nick)
{
	if (isOperator(nick))
	{
		std::vector<std::string>::iterator it;
		it = std::find(operators.begin(), operators.end(), nick);
		operators.erase(it);
	}
	else
		std::cout << "client is not an operator" << std::endl;
}

bool	channel::isOperator(std::string nick)
{
	std::vector<std::string>::iterator it;
	it = std::find(operators.begin(), operators.end(), nick);
	if (it != operators.end())
		return true;
	return false;
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
