#include "server.hpp"
#include <iostream>
#include <stdlib.h>

server constructServer(std::string port, std::string password)
{
	if (port.empty() || password.empty())
		throw std::runtime_error("Invalid Arguments");
	for (size_t i = 0; i < port.length(); i++)
		if (!isdigit(port[i]))
			throw std::runtime_error("The port contains non-digits");
	unsigned int p = atoi(port.c_str());
	if (p > 65535)
		throw std::runtime_error("The port is invalid");
	return server(p, password);
}

int main(int ac, char *av[])
{
	if (ac != 3)
		return 1;
	try
	{
		server srv = constructServer(av[1], av[2]);
		srv.startServer();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

// connection
// registration
// channel operations
// signals
// BONUS
