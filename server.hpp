#ifndef SERVER_HPP
# define SERVER_HPP

#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include "client.hpp"
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

class server {
	private:
		int port;
		std::string password;
		int s_socket;
		std::vector <struct pollfd> poll_fds;
		std::vector <client> clients;
		std::map<int, std::string> read_buffer;
	public:
		server(int p, std::string pass);
		~server();
		void startServer();
		void execute_cmds(int fd);
		void authenticate_cmds(std::string line, int fd);
};

#endif
