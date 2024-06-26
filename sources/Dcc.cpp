#include "Dcc.hpp"
#include "Bot_utils.hpp"
#include "Client.hpp"
// #include "Server.hpp"

// #include <algorithm>
#include <iostream>

Dcc::Dcc(client clt) : clt(clt) {
  this->foundCltFd = -1;
  //
}

void Dcc::setData(std::string message, std::vector<client*> clients) {
  this->cltMessage = message;
  this->clients = clients;
  this->parseMessage(); // calling the parser
}

void Dcc::parseMessage() {
  this->clientResponce.erase();
  std::string name;
  std::cout << "Message set by client(" << this->clt.getUsername()
            << "): " << this->cltMessage << " ." << std::endl;
  // search for user if available
  std::vector<std::string> keys = splitString(this->cltMessage, ' ');

  name = keys.at(1);

  std::cout << "User: " << name << std::endl;

  if (!this->findClient(name) && this->clt.getNickname() != name) {
    this->clientResponce = ":localhost 401 " + this->clt.getNickname() + " " +
                           name + " :No such nick/channel\r\n";
    this->foundCltFd = -1;
    return;
  }

  this->clientResponce = ":" + this->clt.getNickname() + "!" +
                         this->clt.getUsername() + "@localhost " +
                         this->cltMessage + "\r\n";
}

int Dcc::getCltFd() const {
  if (this->foundCltFd != -1)
    return this->foundCltFd;
  else
    return this->clt.getFd();
}

/*
 * @Dcc::client looks if client exist or not
 * */
bool Dcc::findClient(std::string name) {

  for (size_t i = 0; i < this->clients.size(); i++) {
    if (clients[i]->getNickname() == name) {
      this->foundCltFd = clients[i]->getFd();
      return true;
    }
  }
  return false;
}

std::string Dcc::getClientResponce() const { return this->clientResponce; }

Dcc::~Dcc() {}
