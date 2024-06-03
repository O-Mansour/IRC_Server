#include "client.hpp"
#include <string>

client::client(int nb, std::string ip_addr) : fd(nb), ip(ip_addr)
{
	for (int i = 0; i < 3; i++)
		authentication[i] = false;
}

client::~client()
{
}

std::string client::getNickname() const
{
	return nickname;
}

void client::setNickname(const std::string n)
{
	nickname = n;
}
