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

void server::deleteClientData(client& clt)
{
	// erase him from poll_fds vector
	for (size_t i = 0; i < poll_fds.size(); i++)
	{
		if (poll_fds[i].fd == clt.getFd())
			poll_fds.erase(poll_fds.begin() + i);
	}
	// erase him from clients vector
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == clt.getFd())
			clients.erase(clients.begin() + i);
	}
	// erase his read_buffer
	std::map<int, std::string>::iterator it = read_buffer.find(clt.getFd());
	if (it != read_buffer.end())
		read_buffer.erase(it);
	// remove him from all channels
	for (size_t i = 0; i < channels.size(); i++)
	{
		int uIndex = channels[i].getUserIndex(clt.getNickname());
		if (uIndex != NOT_VALID)
			channels[i].remove_user(uIndex, clt.getNickname());
	}
	// close his fd
	close(clt.getFd());
}

void server::send_reply(int fd, std::string str){
	send(fd, str.c_str(), str.length(), 0);
}

void server::HandleData(client& clt)
{
	char buf[BUFFER_SIZE];
	ssize_t rn = recv(clt.getFd(), buf, BUFFER_SIZE - 1, 0);
	if (rn < 0)
		throw std::runtime_error("An error occurred with recv()");
	else if (rn == 0)
	{
		print_time();
		std::cout << RED << "Client with fd : " << clt.getFd() << " disconnected" << RESET << std::endl;
		deleteClientData(clt);
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
	std::string reply;
	if (command.size() < 2)
		send_reply(clt.getFd(), (ERR_NEEDMOREPARAMS()));
	else {
		if(clt.authentication[0])
			send_reply(clt.getFd(), (ERR_ALREADYREGISTERED(clt.getNickname())));
		else {
			if (!command[1].compare(password))
				clt.authentication[0] = true;
			else
				send_reply(clt.getFd(), (ERR_PASSWDMISMATCH()));
		}
	}
}

int ValidNick(std::string &str){
	if (str[0] == '#' || str[0] == '$'|| str[0] == '&' || str[0] == ':')
		return 0;
	for (size_t i = 0; i < str.length(); i++){
		if ((str[i] >= 9 && str[i] <= 13) || str[i] == 32 || str[i] == ',' || str[i] == '*'
			|| str[i] == '?' || str[i] == '!' || str[i] == '@' || str[i] == '.')
			return 0;
	}
	return 1;
}

void server::check_nickname(std::vector<std::string>& command, client& clt){
	std::string reply;
	if (command.size() != 2 && command.size() != 3)
		send_reply(clt.getFd(), ERR_NONICKNAMEGIVEN(clt.getNickname()));
	else if (!ValidNick(command[1]))
		send_reply(clt.getFd(), ERR_ERRONEUSNICKNAME());
	else if (command.size() == 3){
		if (!ValidNick(command[2]))
			send_reply(clt.getFd(), ERR_ERRONEUSNICKNAME());
		else if (clt.authentication[1]){
			if (!clt.getNickname().compare(command[1])){
				clt.setNickname(command[2]);
				send_reply(clt.getFd(), ERR_NICKNAMECHANGE(command[1], command[2]));
			}
			else
				send_reply(clt.getFd(), ERR_NICKNAMEINVALID(command[1]));
		}
		else
			send_reply(clt.getFd(), ERR_NICKNAMEFIRST());
	}
	else {
		size_t i;
		for (i = 0; i < clients.size(); i++) {
			if (!clients[i].getNickname().compare(command[1]))
			{
				send_reply(clt.getFd(), ERR_NICKNAMEINUSE(clt.getNickname(), command[1]));
				break;
			}
		}
		if (i == clients.size())
		{
			clt.setNickname(command[1]);
			clt.authentication[1] = true;
		}
	}
}

void server::check_username(std::vector<std::string>& command, client& clt, std::string &line) {
	if (command.size() < 5)
		return send_reply(clt.getFd(), ERR_NEEDMOREPARAMS());
	else if (clt.authentication[2])
		return send_reply(clt.getFd(), ERR_ALREADYREGISTERED(clt.getNickname()));
	else if(command[2].compare("0") || command[3].compare("*"))
		return send_reply(clt.getFd(), ERR_USERFORMAT());
	else if (command[4].at(0) != ':')
		return send_reply(clt.getFd(), ERR_USERSYNTAX());
	else{
		if (command.size() == 5){
			clt.setUsername(command[1]);
			if (command[4].size() < 2)
				return send_reply(clt.getFd(), ERR_USERSYNTAX());	
			clt.setFullname(command[4].substr(1));
			clt.authentication[2] = true;
		}
		else{
				// it looks a repetition
				clt.setUsername(command[1]);
				if (command[4].size() < 2)
					return send_reply(clt.getFd(), ERR_USERSYNTAX());
				line = extract_param(command, line, 4);
				clt.setFullname(line);
				clt.authentication[2] = true;
		}
		std::string one = ":localhost.irc.com 001 "+ clt.getNickname() +" :Welcome to the LEET Network4, " + clt.getNickname() + "\r\n";
		std::string two = ":localhost.irc.com 002 IRSSI :Your host is localhost, running version 1.0\r\n";
		std::string three = ":localhost.irc.com 003 IRSSI :This server was created Sun Jun 09 17:19:15 2024\r\n";
		std::string four = ":localhost.irc.com 004 IRSSI localhost 1.0 CHANTYPES=#\r\n";
		std::string five = ":localhost.irc.com 005 CHANTYPES=# NICKLEN=16 CASEMAPPING=rfc1459 CHANMODES=k,l,it MODES=1 :are supported\r\n";
		std::string motd = ":localhost.irc.com 422 IRSSI :MOTD is missing\r\n";
		send_reply(clt.getFd(), one);
		send_reply(clt.getFd(), two);
		send_reply(clt.getFd(), three);
		send_reply(clt.getFd(), four);
		send_reply(clt.getFd(), five);
		send_reply(clt.getFd(), motd);
	}
}

void server::authenticate_cmds(std::string line, client& clt)
{
	std::string reply;
	std::vector<std::string> command = split_line(line);
	if (!command[0].compare("PASS"))
		check_password(command, clt);
	if (!command[0].compare("NICK") && clt.authentication[0])
		check_nickname(command, clt);
	else if (!command[0].compare("NICK")){
		reply = ERR_NEEDMOREPARAMS();
		send(clt.getFd(), reply.c_str(), reply.length(), 0);
	}
	if (!command[0].compare("USER") && clt.authentication[0] && clt.authentication[1])
		check_username(command, clt, line);
	else if (!command[0].compare("USER")){
		reply = ERR_NEEDMOREPARAMS();
		send(clt.getFd(), reply.c_str(), reply.length(), 0);
	}
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
		command[2].erase(0, 1);
		for (size_t i = 0; i < clients.size(); i++){
			if (!clients[i].getNickname().compare(command[1])){
				user_exist = true;
				fd = clients[i].getFd();
			}
		}
		if (!user_exist){
			std::cout << "This user : " << command[1] << " doesn't exist" << std::endl;
			return ;
		}
		for (size_t i = 0; i < channels.size(); i++){
			if (!channels[i].getName().compare(command[2]) && channels[i].getSize() > 0)
				channel_exist = true;
		}
		if (!channel_exist){
			std::cout << "This channel : " << command[2] << " doesn't exist" << std::endl;
			return ;
		}
		for (size_t i = 0; i < channels.size(); i++){
			if (channels[i].getCltFd(clt.getFd()))
				clt_part_in_it = true;
			if (channels[i].getCltFd(fd)){
				ss << RED << "You already joined this channel : " << command[2] << RESET << std::endl;
				write(fd, ss.str().c_str(), ss.str().size());
				return ;
			} 
			else if (channels[i].c_modes[INVITE_ONLY_M] == false){
				ss << RED << "This channel has INVITE_ONLY mode : " << command[2] << RESET << std::endl;
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
		size_t i = 0;
		for (; i < channels.size(); i++){
			if (!channels[i].getName().compare(command[1])){
				if (!channels[i].isOperator(clt.getNickname())){
					std::cout << "You are not an operator" << std::endl;
					return ;
				}
				int j = channels[i].getUserIndex(command[2]);
				if (j == -1){
					std::cout << RED << "This User doesn't exist in this channel" << RESET << std::endl;
					return;
				}
				ss << RED << clt.getNickname() << " Kicked your ass from the channel" << RESET << std::endl;;
				if (command.size() == 4)
					ss << UNDERLINE << "Reason: " << RESET << command[3] << std::endl;
				write(channels[i].user_fd(command[2]), ss.str().c_str(), ss.str().size());
				channels[i].remove_user(j, command[2]);
				return ;
			}
		}
		std::cout << RED << "This channel doesn't exist" << RESET << std::endl;
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

void server::send_pong(std::vector<std::string> &command, client &clt)
{
	if (command.size() < 2 || command[1].empty())
		send_reply(clt.getFd(), (ERR_NEEDMOREPARAMS()));
	else
		send_reply(clt.getFd(), "PONG " + std::string(SERVER_NAME) + " " + command[1] + "\r\n");
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
	else if (!command[0].compare("PING"))
		send_pong(command, clt);
}

void server::execute_cmds(client& clt)
{
	size_t pos;
	std::string line;
	// may need to implement \r\n as end of cmd
	while ((pos = read_buffer[clt.getFd()].find("\n")) != std::string::npos)
	{
		line = read_buffer[clt.getFd()].substr(0, pos);
		std::cout << YELLOW << "line to parse : " << RESET;
		std::cout << line << std::endl;
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

