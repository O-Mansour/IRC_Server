#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Client.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

# define INVITE_ONLY_M 0
# define TOPIC_RESTRICTION_M 1
# define CHANNEL_KEY_M 2
# define USER_LIMIT_M 3

class channel {
	private:
		std::string name;
		std::string topic;
		std::string key;
		size_t userLimit;
		std::vector <client> clients;
		std::vector <std::string> operators;
	public:
		channel(std::string n, std::string opr);
		~channel();
		bool c_modes[4];
		std::string getName() const;
		std::string getTopic() const;
		void		setTopic(const std::string t);
		size_t		getUserLimit() const;
		void		setUserLimit(const size_t limit);
		std::string getKey() const;
		void		setKey(const std::string t);
		void		addAsOperator(std::string nick);
		void		eraseOperator(std::string nick);
		void		c_join(client &clt, std::string k);
		bool		isOperator(std::string nick);
};

#endif
