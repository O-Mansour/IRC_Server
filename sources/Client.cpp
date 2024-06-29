#include "Client.hpp"

client::client(int nb) : fd(nb) {
  for (int i = 0; i < 3; i++)
    authentication[i] = false;
}

client::~client() {}

int client::getFd() const { return fd; }

void client::setNickname(const std::string n) { nickname = n; }

std::string client::getNickname() const { return nickname; }

void client::setUsername(const std::string n) { username = n; }

std::string client::getUsername() const { return username; }

void client::setFullname(const std::string n) { fullname = n; }

std::string client::getFullname() const { return nickname; }
