#pragma once

#include "Client.hpp"
#include <vector>

// typedef std::vector<client*>::iterator clt_it;

class Dcc {
private:
  std::string clientResponce;
  client clt;
  int foundCltFd;
  std::string cltMessage;
  std::vector<client*> clients;

public:
  Dcc(client clt);
  ~Dcc();

  void parseMessage();
  void setData(std::string message, std::vector<client*> clients);
  bool findClient(std::string name);
  int getCltFd() const;

  std::string getClientResponce() const;
};
