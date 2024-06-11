#include "includes/Server.hpp"

server constructServer(std::string port, std::string password)
{
	if (port.empty() || password.empty())
		throw std::runtime_error("Empty Arguments");
	if (port.length() > 5)
		throw std::runtime_error("The port is invalid");
	for (size_t i = 0; i < port.length(); i++)
	{
		if (!isdigit(port[i]))
			throw std::runtime_error("The port contains non-digits");
	}
	unsigned int p = std::atoi(port.c_str());
	if (p > MAX_UINT16 || p < RESERVED_PORTS)
		throw std::runtime_error("The port is invalid");
	print_ft_irc();
	return server(p, password);
}

int main(int ac, char *av[])
{
	if (ac != 3)
	{
		std::cerr << RED << "The ircserv needs port and password" << RESET << std::endl;
		std::cerr << "For example : ./ircserv 6667 pass" << std::endl;
		return 1;
	}
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

// for replies
// :server_name <numeric reply code> <message text>
