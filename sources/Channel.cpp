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

void channel::c_join(client &clt, std::string key)
{
	(void) clt;
	(void) key;

	// to be continued
}
