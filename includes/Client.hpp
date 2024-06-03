#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
class client {
	private:
		int fd;
		std::string ip;
		std::string nickname;
	public:
		client(int nb, std::string ip_addr);
		~client();
		bool authentication[3];
		std::string getNickname() const;
		void setNickname(const std::string n);
};

#endif
