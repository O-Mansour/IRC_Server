#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
class client {
	private:
		int fd;
		std::string ip;
		std::string nickname;
		std::string username;
		std::string fullname;
	public:
		client(int nb, std::string ip_addr);
		~client();
		bool authentication[3];
		std::string getNickname() const;
		void setNickname(const std::string n);
		std::string getUsername() const;
		void setUsername(const std::string n);
		std::string getFullname() const;
		void setFullname(const std::string n);
};

#endif
