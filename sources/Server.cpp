#include "../includes/Server.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

server::server(int p, std::string pass) : port(p), password(pass)
{
	//creating a simple tcp server
	if ((this->s_socket = socket(AF_INET, SOCK_STREAM, 0)) == NOT_VALID)
		throw std::runtime_error("socket() failed");
	struct sockaddr_in s_addr;
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = INADDR_ANY;
	int optval = 1;
	setsockopt(this->s_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == NOT_VALID)
		throw std::runtime_error("bind() failed");
	if (listen(this->s_socket, SOMAXCONN) == NOT_VALID)
		throw std::runtime_error("listen() failed");
	// start poll() 
	struct pollfd to_poll;
	to_poll.fd = this->s_socket;
	to_poll.events = POLLIN;
	this->poll_fds.push_back(to_poll);
}

server::~server() {}

void server::CreateClient(){
	struct sockaddr_in clt_addr;
	socklen_t addr_len = sizeof(clt_addr);
	int nb = accept(s_socket, (sockaddr *) &clt_addr, &addr_len);
	if (nb == NOT_VALID)
		throw std::runtime_error("accept() failed");
	struct pollfd clt_fd;
	clt_fd.fd = nb;
	clt_fd.events = POLLIN;
	client clt(nb, inet_ntoa(clt_addr.sin_addr));
	clients.push_back(clt);
	poll_fds.push_back(clt_fd);
	print_time();
	std::cout << YELLOW << "New client connected with fd : " << nb << RESET << std::endl;
}

void server::HandleData(int i){
	char buf[BUFFER_SIZE];
	ssize_t rn = read(i, buf, BUFFER_SIZE - 1);
	if (rn < 0)
		throw std::runtime_error("read() failed");
	else if (rn == 0)
	{
		print_time();
		std::cout << RED << "Client with fd : " << i << " disconnected" << RESET << std::endl;
		close(i);
	}
	else {
		buf[rn] = '\0';
		read_buffer[i].append(buf);
		execute_cmds(i);
	}

}

void server::startWaiting()
{
	while (true)
	{
		if (poll(&poll_fds[0], poll_fds.size(), -1) == -1)
			throw std::runtime_error("poll() failed");
		for(size_t i = 0 ; i < poll_fds.size(); i++)
		{
			if ((poll_fds[i].revents & POLLIN))
			{
				if (poll_fds[i].fd == s_socket)
					this->CreateClient();
				else
					this->HandleData(poll_fds[i].fd);
			}
		}
	}
}

std::vector<std::string> split_line(std::string line)
{
	std::vector<std::string> res;
	char* word;

	word = std::strtok(const_cast<char*>(line.c_str()), " 	");
	while (word)
	{
		res.push_back(word);
		word = std::strtok(NULL, " 	");
	}
	// res.push_back(line);
	return res;
}

void server::check_password(std::vector<std::string> &command, int fd){
	if (command.size() != 2)
		std::cout << RED << "number of args isn't right" << RESET << std::endl;
	else {
		if(clients[fd].authentication[0])
			std::cout << RED << "You entered the password already" << RESET<< std::endl;
		else {
			if (!command[1].compare(password))
			{
				std::cout << GREEN << "Correct password, Welcome" << RESET << std::endl;
				clients[fd].authentication[0] = true;
			}
			else
				std::cout << RED << "Incorrect password" << RESET << std::endl;
		}
	}
}

void server::check_nickname(std::vector<std::string>& command, int fd){
	if (command.size() != 2)
		std::cout << RED << "number of args isn't right" << RESET << std::endl;
	else {
		bool nickAvailable = true;
		for (size_t i = 0; i < clients.size(); i++) {
			if (!clients[i].getNickname().compare(command[1]))
			{
				nickAvailable = false;
				std::cout << YELLOW << "nickname is already used, Please try another one" << RESET << std::endl;
				break;
			}
		}
		if (nickAvailable)
		{
			clients[fd].setNickname(command[1]);
			clients[fd].authentication[1] = true;
			std::cout << GREEN << "Nickname added successfully, Welcome " << RESET << BOLD << clients[fd].getNickname() << RESET << std::endl;
		}
	}
}

void server::check_username(std::vector<std::string>& command, int fd){
	(void)fd;
	if (command.size() < 5){
		std::cout << RED << "Number of args isn't right, Please use this syntax : " << RESET << std::endl;
		std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
	}
	else{
		if(command[2].compare("0") || command[3].compare("*"))
			std::cout << UNDERLINE << "Please set the second parameter with <0> and the third with <*>" << RESET << std::endl;
		else{
			// clients[fd].setUsername(command[1]);
			// clients[fd].authentication[2] = true;
		}
		// size_t pos = command[4].find(":");
		// if (pos == std::string::npos)
		// 	std::cout << RED << "Didn't find :" << RESET << std::endl;
		// else{
		// 	clients[fd].setFullname(command[4].substr(pos));
		// 	std::cout << GREEN << "Username added successfully" << RESET << std::endl;
		// }
	}

}

void server::authenticate_cmds(std::string line, int fd)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("PASS"))
		check_password(command, fd);
	if (!command[0].compare("NICK") && clients[fd].authentication[0])
		check_nickname(command, fd);
	else if (!command[0].compare("NICK") && !clients[fd].authentication[0])
		std::cout << UNDERLINE << "Please enter the password first" << RESET << std::endl;
	if (!command[0].compare("USER") && clients[fd].authentication[0] && clients[fd].authentication[1])
		check_username(command, fd);
	else if (!command[0].compare("USER") && (!clients[fd].authentication[0] || !clients[fd].authentication[1]))
		std::cout << UNDERLINE << "You need first to set <PASS> and <NICK>" << RESET << std::endl;
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

void print_time(){
    std::time_t currentTime = std::time(NULL);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "["
		<< std::setw(4) << localTime->tm_year + 1900 << "-"
		<< std::setw(2) << std::setfill('0') << localTime->tm_mon + 1 << "-"
		<< std::setw(2) << std::setfill('0') << localTime->tm_mday << " "
		<< std::setw(2) << std::setfill('0') << localTime->tm_hour << ":"
		<< std::setw(2) << std::setfill('0') << localTime->tm_min << ":"
		<< std::setw(2) << std::setfill('0') << localTime->tm_sec << "]" 
		<< std::endl;
}


void print_ft_irc(){
	std::cout << GREEN << "███████╗████████╗     ██╗██████╗  ██████╗" << RESET << std::endl;
	std::cout << GREEN << "██╔════╝╚══██╔══╝     ██║██╔══██╗██╔════╝" << RESET << std::endl;
	std::cout << GREEN << "█████╗     ██║        ██║██████╔╝██║     " << RESET << std::endl;
	std::cout << GREEN << "██╔══╝     ██║        ██║██╔══██╗██║     " << RESET << std::endl;
	std::cout << GREEN << "██║        ██║███████╗██║██║  ██║╚██████╗" << RESET << std::endl;
	std::cout << GREEN << "╚═╝        ╚═╝╚══════╝╚═╝╚═╝  ╚═╝ ╚═════╝" << RESET << std::endl;
	std::cout << GREEN << "                                         " << RESET << std::endl;
}

