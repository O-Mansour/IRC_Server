#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
class client {
private:
  int fd;
  std::string nickname;
  std::string username;
  std::string fullname;

public:
  client(int nb);
  ~client();
  bool authentication[3];
  int getFd() const;
  std::string getNickname() const;
  std::string getUsername() const;
  std::string getFullname() const;
  void setNickname(const std::string n);
  void setUsername(const std::string n);
  void setFullname(const std::string n);
};

#endif
