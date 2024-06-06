#include "includes/Server.hpp"

server constructServer(std::string port, std::string password)
{
	if (port.empty() || password.empty())
		throw std::runtime_error("empty Arguments");
	for (size_t i = 0; i < port.length(); i++)
		if (!isdigit(port[i]))
			throw std::runtime_error("The port contains non-digits");
	unsigned int p = atoi(port.c_str());
	if (p > MAX_UINT16 || p < RESERVED_PORTS)
		throw std::runtime_error("The port is invalid");
	print_ft_irc();
	return server(p, password);
}

int main(int ac, char *av[])
{
	if (ac != 3)
		return 1;
	try
	{
		server srv = constructServer(av[1], av[2]);
		srv.start();
	}
	catch (std::exception &e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
	return 0;
}

// connection
// registration
// channel operations
// signals
// BONUS
