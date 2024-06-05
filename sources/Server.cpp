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
	// setup for poll()
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
	poll_fds.push_back(clt_fd);
	client clt(nb);
	clients.push_back(clt);
	print_time();
	std::cout << YELLOW << "New client connected with fd : " << nb << RESET << std::endl;
}

void server::HandleData(client& clt)
{
	char buf[BUFFER_SIZE];
	ssize_t rn = read(clt.getFd(), buf, BUFFER_SIZE - 1);
	if (rn < 0)
		throw std::runtime_error("read() failed");
	else if (rn == 0)
	{
		print_time();
		std::cout << RED << "Client with fd : " << clt.getFd() << " disconnected" << RESET << std::endl;
		close(clt.getFd());
	}
	else {
		buf[rn] = '\0';
		read_buffer[clt.getFd()].append(buf);
		execute_cmds(clt);
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
					this->HandleData(clients[i - 1]);
			}
		}
	}
}

// needs more tests
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

void server::check_password(std::vector<std::string> &command, client& clt){
	if (command.size() != 2)
		std::cout << RED << "number of args isn't right" << RESET << std::endl;
	else {
		if(clt.authentication[0])
			std::cout << RED << "You entered the password already" << RESET<< std::endl;
		else {
			if (!command[1].compare(password))
			{
				std::cout << GREEN << "Correct password, Welcome" << RESET << std::endl;
				clt.authentication[0] = true;
			}
			else
				std::cout << RED << "Incorrect password" << RESET << std::endl;
		}
	}
}

void server::check_nickname(std::vector<std::string>& command, client& clt){
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
			clt.setNickname(command[1]);
			clt.authentication[1] = true;
			std::cout << GREEN << "Nickname added successfully, Welcome " << RESET << BOLD << clt.getNickname() << RESET << std::endl;
		}
	}
}

void server::check_username(std::vector<std::string>& command, client& clt) {
	if (command.size() != 5){
		std::cout << RED << "Number of args isn't right, Please use this syntax : " << RESET << std::endl;
		std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
	}
	else{
		if(command[2].compare("0") || command[3].compare("*"))
			std::cout << UNDERLINE << "Please set the second parameter with <0> and the third with <*>" << RESET << std::endl;
		else{
			clt.setUsername(command[1]);
			clt.setFullname(command[4]);
			clt.authentication[2] = true;
            write(clt.getFd(), "You can use all commands\n", 26);
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

void server::authenticate_cmds(std::string line, client& clt)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("PASS"))
		check_password(command, clt);
	else if (!command[0].compare("NICK") && clt.authentication[0])
		check_nickname(command, clt);
	else if (!command[0].compare("NICK"))
		std::cout << UNDERLINE << "Please enter the password first" << RESET << std::endl;
	else if (!command[0].compare("USER") && clt.authentication[0] && clt.authentication[1])
		check_username(command, clt);
	else if (!command[0].compare("USER"))
		std::cout << UNDERLINE << "You need first to set <PASS> and <NICK>" << RESET << std::endl;
}

void server::do_join(std::vector<std::string> &command, client &clt)
{
	if ((command.size() != 2 && command.size() != 3) || command[1].at(0) != '#')
			std::cout << "args are invalid" << std::endl;
	else
	{
		command[1].erase(0, 1);
		bool channelAvailable = false;
		std::string c_key;
		if (command.size() == 3)
			c_key = command[2];
		for (size_t i = 0; i < channels.size(); i++) {
			if (!channels[i].getName().compare(command[1]))
			{
				channelAvailable = true;
				channels[i].c_join(clt, c_key);
				std::cout << "user has joined the channel" << std::endl;
				break;
			}
		}
		if (!channelAvailable)
		{
			// create channel and add the user to it
			channel cnl(command[1]);
			cnl.c_join(clt, "");
			channels.push_back(cnl);
		}
	}
}

void server::do_topic(std::vector<std::string> &command, client &clt)
{
	// need to check the mode (t) here
	if (command.size() == 2 && command[1].at(0) != '#')
	{
		// TOPIC #channelname
		command[1].erase(0, 1);
		size_t i;
		for (i = 0; i < channels.size(); i++) {
			if (!channels[i].getName().compare(command[1]))
			{
				std::string c_topic = channels[i].getTopic();
				send(clt.getFd(), c_topic.c_str(), c_topic.length(), 0);
				break;
			}
		}
		if (i == channels.size())
			std::cout << "Channel doesn't exist" << std::endl;
	}
	// TOPIC #channel:::name :The New     Topic
	// TOPIC #channelname :ThisIsTheNewTopic
	else if (command.size() > 2 && command[1].at(0) != '#')
	{}
	// now i am working here
}

void server::channel_cmds(std::string line, client& clt)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("/JOIN"))
		do_join(command, clt);
	else if (!command[0].compare("/TOPIC"))
		do_topic(command, clt);
}

void server::execute_cmds(client& clt)
{
	size_t pos;
	std::string line;
	while ((pos = read_buffer[clt.getFd()].find("\n")) != std::string::npos)
	{
		line = read_buffer[clt.getFd()].substr(0, pos);
		authenticate_cmds(line, clt);
		if (clt.authentication[0] && clt.authentication[1] && clt.authentication[2])
			channel_cmds(line, clt);
		read_buffer[clt.getFd()].erase(0, pos + 1);
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

