#include "server.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

server::server(int p, std::string pass) : port(p), password(pass)
{
	if ((s_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw std::runtime_error("socket() failed");
	struct sockaddr_in s_addr;
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = INADDR_ANY;
	int optval = 1;
	setsockopt(s_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1)
		throw std::runtime_error("bind() failed");
	listen(s_socket, SOMAXCONN);

	struct pollfd to_poll;
	to_poll.fd = s_socket;
	to_poll.events = POLLIN;
	poll_fds.push_back(to_poll);
}

server::~server() {}

void server::startServer()
{
	while (true)
	{
		if (poll(&poll_fds[0], poll_fds.size(), -1) == -1)
			throw std::runtime_error("poll() failed");
		for(size_t i = 0 ; i < poll_fds.size(); i++)
		{
			if ((poll_fds[i].revents & POLLIN))
			{
				// first, accept clients
				if (poll_fds[i].fd == s_socket) {
					struct sockaddr_in clt_addr;
					socklen_t addr_len = sizeof(clt_addr);
					int nb = accept(s_socket, (sockaddr *) &clt_addr, &addr_len);
					if (nb == -1)
						throw std::runtime_error("accept() failed");
					struct pollfd clt_fd;
					clt_fd.fd = nb;
					clt_fd.events = POLLIN;
					client clt(nb, inet_ntoa(clt_addr.sin_addr));
					clients.push_back(clt);
					poll_fds.push_back(clt_fd);
					std::cout << "new client connected, fd :" << nb << std::endl;
				}
				// parse the data
				else {
					char buf[BUFFER_SIZE];
					ssize_t rn = read(poll_fds[i].fd, buf, BUFFER_SIZE - 1);
					if (rn < 0)
						throw std::runtime_error("read() failed");
					else if (rn == 0)
					{
						std::cout << "client disconnected" << std::endl;
						close(poll_fds[i].fd);
					}
					else {
						buf[rn] = '\0';
						read_buffer[poll_fds[i].fd].append(buf);
						execute_cmds(poll_fds[i].fd);
					}
				}
			}
		}
	}
}

std::vector<std::string> split_line(std::string line)
{
	std::vector<std::string> res;
	std::size_t pos;
	std::string word;

	while ((pos = line.find(" ")) != std::string::npos)
	{
		word = line.substr(0, pos);
		res.push_back(word);
		line.erase(0, pos + 1);
	}
	res.push_back(line);
	return res;
}

void server::authenticate_cmds(std::string line, int fd)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("PASS")) {
		if (command.size() != 2)
			std::cout << "number of args isn't right" << std::endl;
		else {
			if(clients[fd].authentication[0])
				std::cout << "entered password already" << std::endl;
			else {
				if (!command[1].compare(password))
				{
					std::cout << "correct password, wassup :)" << std::endl;
					clients[fd].authentication[0] = true;
				}
				else
					std::cout << "incorrect password" << std::endl;
			}
		}
	}
	else if (!command[0].compare("NICK") && clients[fd].authentication[0]) {
		if (command.size() != 2)
			std::cout << "number of args isn't right" << std::endl;
		else {
			bool nickAvailable = true;
			for (int i = 0; i < clients.size(); i++) {
				if (!clients[i].getNickname().compare(command[1]))
				{
					nickAvailable = false;
					std::cout << "nickname is already in use, try another one" << std::endl;
					break;
				}
			}
			if (nickAvailable)
			{
				clients[fd].setNickname(command[1]);
				clients[fd].authentication[1] = true;
				std::cout << "nickname added successfully" << std::endl;
			}
		}
	}
	else if (!command[0].compare("USER") && clients[fd].authentication[0] 
				&& clients[fd].authentication[1])
	{
		if (command.size() != 4)
			std::cout << "number of args isn't right" << std::endl;
		else
		{
			
		}
	}
}

void server::execute_cmds(int fd)
{
	size_t pos;
	std::string line;
	while ((pos = read_buffer[fd].find("\n")) != std::string::npos)
	{
		line = read_buffer[fd].substr(0, pos);
		authenticate_cmds(line, fd);
		read_buffer[fd].erase(0, pos + 1);
	}
}
