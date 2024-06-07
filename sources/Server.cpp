#include "../includes/Server.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
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

void server::start()
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

std::string extract_param(std::vector<std::string> &command, std::string line, int argIndex)
{
	size_t pos;
	for (int i = 0; i < argIndex; i++)
	{
		pos = line.find(command[i]);
		if (pos != std::string::npos)
			line.erase(0, pos + command[i].length());
	}
	pos = line.find(':');
	if (pos != std::string::npos)
		line.erase(0, pos + 1);
	return line;
}


std::vector<std::string> split_line(std::string line)
{
	std::vector<std::string> res;
	char *buff = new char[line.size() + 1];
	char* word;

	buff = std::strcpy(buff, line.c_str());
	word = std::strtok(buff, " 	\r");
	while (word)
	{
		res.push_back(word);
		word = std::strtok(NULL, " 	\r");
	}
	delete [] buff;
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
	else if (clt.authentication[1])
		std::cout << YELLOW << "You already assigned a nickname" << RESET << std::endl;
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

void server::check_username(std::vector<std::string>& command, client& clt, std::string &line) {
	if (command.size() < 5){
		std::cout << RED << "Number of args isn't right, Please use this syntax : " << RESET << std::endl;
		std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
	}
	else if (clt.authentication[2])
		std::cout << YELLOW << "You already assigned a username" << RESET << std::endl;
	else if(command[2].compare("0") || command[3].compare("*"))
		std::cout << UNDERLINE << "Please set the second parameter with <0> and the third with <*>" << RESET << std::endl;
	else if (command[4].at(0) != ':'){
		std::cout << RED << "Please use this syntax : " << RESET << std::endl;
		std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
		return ;
	}
	else{
		if (command.size() == 5){
			clt.setUsername(command[1]);
			if (command[4].size() < 2){
				std::cout << RED << "Please use this syntax : " << RESET << std::endl;
				std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
				return ;
			}
			clt.setFullname(command[4].substr(1));
			clt.authentication[2] = true;
		 	std::cout << GREEN << "Username added successfully" << RESET << std::endl;
		}
		else{
				clt.setUsername(command[1]);
				if (command[4].size() < 2){
					std::cout << RED << "Please use this syntax : " << RESET << std::endl;
					std::cout << "\t USER <username> 0 * :<realname>" << std::endl;
					return ;
				}
				line = extract_param(command, line, 4);
				clt.setFullname(line);
				clt.authentication[2] = true;
		 		std::cout << GREEN << "Username added successfully" << RESET << std::endl;
			}
	}
}

void server::authenticate_cmds(std::string line, client& clt)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("PASS"))
		check_password(command, clt);
	if (!command[0].compare("NICK") && clt.authentication[0])
		check_nickname(command, clt);
	else if (!command[0].compare("NICK"))
		std::cout << UNDERLINE << "Please enter the password first" << RESET << std::endl;
	if (!command[0].compare("USER") && clt.authentication[0] && clt.authentication[1])
		check_username(command, clt, line);
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
				channels[i].setSize();
				std::cout << "user has joined the channel" << std::endl;
				break;
			}
		}
		if (!channelAvailable)
		{
			// create channel and add the user to it
			channel cnl(command[1], clt.getNickname());
			cnl.c_join(clt, "");
			cnl.setSize();
			channels.push_back(cnl);
		}
	}
}

void server::do_privmsg(std::vector<std::string> &command, client &clt, std::string line)
{
	if (command.size() < 3){
		std::cout << RED << "args are invalid, Use this syntax" << RESET << std::endl;
		std::cout << UNDERLINE << "\t PRIVMSG <channel/user nickname> :<message>" << RESET << std::endl;
	}
	else if (command[2].at(0) != ':')
		std::cout << RED << "Not valid syntax" << RESET << std::endl;
	else{
		if (command[1].at(0) == '#'){
			command[1].erase(0, 1);
			for (size_t i = 0; i < channels.size(); i++) {
				if (!channels[i].getName().compare(command[1]))
				{
					line = extract_param(command, line, 2);
					channels[i].c_privmsg(clt, line);
				}
			}
		}
		else{
			line = extract_param(command, line, 2);
			for (size_t i = 0; i < clients.size(); i++){
				if (!clients[i].getNickname().compare(command[1])){
					std::stringstream ss;
					ss << UNDERLINE << "New message from " << clt.getNickname() << " :" << RESET << std::endl;
					write(clients[i].getFd(), ss.str().c_str(), ss.str().size());
					write(clients[i].getFd(), line.c_str(), line.size());
					write(clients[i].getFd(), "\n", 1);
				}
			}
		}
	}
}

void server::do_invite(std::vector<std::string> &command, client &clt){
	std::stringstream ss;
	if (command.size() != 3)
		std::cout << RED << "Syntax error : /INVITE <nickname> <channel>" << RESET << std::endl;		
	else{
		bool channel_exist = false;
		bool clt_part_in_it = false;
		bool user_exist = false;
		int fd;
		for (size_t i = 0; i < clients.size(); i++){
			if (!clients[i].getNickname().compare(command[1])){
				user_exist = true;
				fd = clients[i].getFd();
			}
		}
		if (!user_exist){
			std::cout << "This user : " << command[1] << " doesn't exist" << std::endl;
			return;
		}
		for (size_t i = 0; i < channels.size(); i++){
			if (!channels[i].getName().compare(command[2]) && channels[i].getSize() > 0)
				channel_exist = true;
		}
		if (!channel_exist){
			std::cout << "This channel : " << command[2] << " doesn't exist" << std::endl;
			return;
		}
		for (size_t i = 0; i < channels.size(); i++){
			if (channels[i].getCltFd(clt.getFd()))
				clt_part_in_it = true;
			if (channels[i].getCltFd(fd)){
				ss << RED << "You already joined this channel : " << command[2] << RESET << std::endl;
				write(fd, ss.str().c_str(), ss.str().size());
				return ;
			}
		}
		if (clt_part_in_it){
			ss << UNDERLINE << clt.getNickname() << " invite you to join " << command[2] << " channel" << RESET << std::endl;
			write(fd, ss.str().c_str(), ss.str().size());
		}
		else
			std::cout << "you are not a part of this channel" << std::endl;
	}
}

void server::do_kick(std::vector<std::string> &command, client &clt){
	std::stringstream ss;
	if (command.size() != 3 && command.size() != 4)
		std::cout << "Not valid args" << std::endl;
	else{
		command[1].erase(0, 1);
		for (size_t i = 0; i < channels.size(); i++){
			if (!channels[i].getName().compare(command[1])){
				int j = channels[i].kick_user(command[2]);
				channels.erase(channels.begin() + j);
				std::cout << RED << clt.getNickname() << " Kick your ass from the channel" << RESET << std::endl;;
				if (command.size() == 4)
					std::cout << UNDERLINE << "Reason: " << RESET << command[3] << std::endl;;
			}
		}
		
	}
}


void server::do_topic(std::vector<std::string> &command, client &clt, std::string line)
{
	// need to check the mode (t) here
	if (command.size() == 2 && command[1].at(0) == '#')
	{
		// TOPIC #channelname
		command[1].erase(0, 1);
		size_t i;
		for (i = 0; i < channels.size(); i++) {
			if (!channels[i].getName().compare(command[1]))
			{
				if (channels[i].getTopic().empty())
					std::cout << "Channel has no topic yet" << std::endl;
				else {
					std::string c_topic = channels[i].getTopic() + "\n";
					send(clt.getFd(), c_topic.c_str(), c_topic.length(), 0);
					send(clt.getFd(), "\n", 1, 0);
				}
				break;
			}
		}
		if (i == channels.size())
			std::cout << "Channel doesn't exist" << std::endl;
	}
	// TOPIC #channel:::name :The: New:     Topic
	// TOPIC #channelname :ThisIsTheNewTopic
	else if (command.size() > 2 && command[1].at(0) == '#' && command[2].at(0) == ':')
	{
		std::string new_topic = extract_param(command, line, 2);
		command[1].erase(0, 1);
		size_t i;
		for (i = 0; i < channels.size(); i++) {
			if (!channels[i].getName().compare(command[1]))
			{
				channels[i].setTopic(new_topic);
				break;
			}
		}
		if (i == channels.size())
			std::cout << "Channel doesn't exist" << std::endl;
	}
}

bool	supported_mode(const std::string& mode)
{
	if (!mode.compare("+i") || !mode.compare("-i")
		|| !mode.compare("+t") || !mode.compare("-t")
		|| !mode.compare("+k") || !mode.compare("-k")
		|| !mode.compare("+o") || !mode.compare("-o")
		|| !mode.compare("+l") || !mode.compare("-l"))
			return true;
	return false;
}

void server::do_mode(std::vector<std::string> &command, client &clt)
{
	if ((command.size() != 3 && command.size() != 4) || command[1].at(0) != '#'
		|| !supported_mode(command[2]))
			std::cout << "args are invalid" << std::endl;
	else
	{
		command[1].erase(0, 1);
		// need to make a function that returns the channel to OPTIMIZE
		if (command[1].empty())
			return ;
		if (command.size() == 3)
		{
			size_t i;
			for (i = 0; i < channels.size(); i++) {
				if (!channels[i].getName().compare(command[1]))
				{
					if (!channels[i].isOperator(clt.getNickname()))
					{
						std::cout << "client must be an operator" << std::endl;
						return ;
					}
					// cases : +i -i +t -t -k -l
					if (!command[2].compare("+i"))
						channels[i].c_modes[INVITE_ONLY_M] = true;
					else if (!command[2].compare("-i"))
						channels[i].c_modes[INVITE_ONLY_M] = false;
					else if (!command[2].compare("+t"))
						channels[i].c_modes[TOPIC_RESTRICTION_M] = true;
					else if (!command[2].compare("-t"))
						channels[i].c_modes[TOPIC_RESTRICTION_M] = false;
					else if (!command[2].compare("-k"))
					{
						channels[i].setKey("");
						channels[i].c_modes[CHANNEL_KEY_M] = false;
					}
					else if (!command[2].compare("-l"))
					{
						channels[i].setUserLimit(0);
						channels[i].c_modes[USER_LIMIT_M] = false;
					}
					else
						std::cout << "MODE does not updated" << std::endl;
					break;
				}
			}
			if (i == channels.size())
				std::cout << "Channel doesn't exist" << std::endl;
		}
		else if (command.size() == 4)
		{
			size_t i;
			for (i = 0; i < channels.size(); i++) {
				if (!channels[i].getName().compare(command[1]))
				{
					if (!channels[i].isOperator(clt.getNickname()))
					{
						std::cout << "client must be an operator" << std::endl;
						return ;
					}
					// cases : +k +l +o -o
					if (!command[2].compare("+k"))
					{
						channels[i].setKey(command[3]);
						channels[i].c_modes[CHANNEL_KEY_M] = true;
					}
					else if (!command[2].compare("+l"))
					{
						// need to check the number
						channels[i].setUserLimit(std::atoi(command[3].c_str()));
						channels[i].c_modes[USER_LIMIT_M] = true;
					}
					else if (!command[2].compare("+o"))
						channels[i].addAsOperator(command[3]);
					else if (!command[2].compare("-o"))
						channels[i].eraseOperator(command[3]);
					else
						std::cout << "MODE does not updated" << std::endl;
					break;
				}
			}
			if (i == channels.size())
				std::cout << "Channel doesn't exist" << std::endl;
		}
	}
}

void server::channel_cmds(std::string line, client& clt)
{
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("/JOIN"))
		do_join(command, clt);
	else if (!command[0].compare("/PRIVMSG"))
		do_privmsg(command, clt, line);
	else if (!command[0].compare("/TOPIC"))
		do_topic(command, clt, line);
	else if (!command[0].compare("/INVITE"))
		do_invite(command, clt);
	else if (!command[0].compare("/KICK"))
		do_kick(command, clt);
	else if (!command[0].compare("/MODE"))
		do_mode(command, clt);
}

void server::execute_cmds(client& clt)
{
	size_t pos;
	std::string line;
	while ((pos = read_buffer[clt.getFd()].find("\n")) != std::string::npos)
	{
		line = read_buffer[clt.getFd()].substr(0, pos);
		if (!line.empty() && line.compare("\r") != 0)
		{
			authenticate_cmds(line, clt);
			if (clt.authentication[0] && clt.authentication[1] && clt.authentication[2])
				channel_cmds(line, clt);
		}
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

