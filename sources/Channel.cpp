#include "../includes/Channel.hpp"
#include <sstream>
#include <string>

channel::channel(std::string n, std::string opr) : name(n), userLimit(0)
{
	this->isBotJoined = false;
	this->size = 0;
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

void channel::setSize(){
	this->size += 1;
}

size_t channel::getSize(){
	return this->size;
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

bool channel::getCltFd(int fd){
	for (size_t i = 0; i < clients.size() ; i++)
		if (clients[i].getFd() == fd)
			return true;
	return false;
}

void channel::c_privmsg(client &clt, std::string key){
	for (size_t i = 0; i < clients.size(); i++){
		std::stringstream ss;
		ss << UNDERLINE << "New message from " << clt.getNickname() << " :" << RESET << std::endl;
		write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
		write(clients[i].getFd(), "#", 1);
		write(clients[i].getFd(), this->getName().c_str(), this->getName().size());
		write (clients[i].getFd(), " : ", 3);
		write(clients[i].getFd(), key.c_str(), key.size());
		write(clients[i].getFd(), "\n", 1);
	}
}

void channel::msgToAllMemebers(std::string nickname, std::string key){
	for (size_t i = 0; i < clients.size(); i++){
		std::stringstream ss;
		ss << UNDERLINE << "New message from " << nickname << " :" << RESET << std::endl;
		write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
		write(clients[i].getFd(), "#", 1);
		write(clients[i].getFd(), this->getName().c_str(), this->getName().size());
		write (clients[i].getFd(), " : ", 3);
		write(clients[i].getFd(), key.c_str(), key.size());
		write(clients[i].getFd(), "\n", 1);
	}
}

int channel::kick_user(std::string &key){
	for (size_t i = 0; i < clients.size(); i++){
		if (!clients[i].getNickname().compare(key))
			return i;
	}
	return -1;
}

int channel::remove_user(std::string &key){
	for (size_t i = 0; i < clients.size(); i++){
		if (!clients[i].getNickname().compare(key))
			clients.erase(clients.begin() + i);
	}
	return -1;
}


int channel::user_fd(std::string &key){
	for (size_t i = 0; i < clients.size(); i++){
		if (!clients[i].getNickname().compare(key))
			return clients[i].getFd();
	}
	return -1;
}

// getters and setter for bot
bool channel::getIsBotJoined() const { return this->isBotJoined; }

void channel::setIsBotJoined(bool val) { this->isBotJoined = val; }
