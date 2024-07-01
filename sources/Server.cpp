#include "Server.hpp"

server::server(int p, std::string pass)
    : port(p), password(pass), bot("lhaj", "lhaj Molshi", "lhaj") {
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
  struct pollfd to_poll;
  to_poll.fd = this->s_socket;
  to_poll.events = POLLIN;
  this->poll_fds.push_back(to_poll);
}

server::~server() {}

void server::CreateClient() {
  struct sockaddr_in clt_addr;
  socklen_t addr_len = sizeof(clt_addr);
  int nb = accept(s_socket, (sockaddr *)&clt_addr, &addr_len);
  if (nb == NOT_VALID)
    throw std::runtime_error("accept() failed");
  struct pollfd clt_fd;
  clt_fd.fd = nb;
  clt_fd.events = POLLIN;
  poll_fds.push_back(clt_fd);
  client* clt = new client(nb);
  clients.push_back(clt);
  print_time();
  std::cout << YELLOW << "New client connected with fd : " << nb << RESET << std::endl;
}

void server::deleteClientData(client *clt) {
  print_time();
  std::cout << RED << "Client with fd : " << clt->getFd() << " disconnected" << RESET << std::endl;
  for (size_t i = 0; i < channels.size(); i++) {
    // remove invitations
    int inviteIndex = channels[i].getInviteIndex(clt->getFd());
    if (inviteIndex != NOT_VALID)
      channels[i].removeFromInvitedFds(inviteIndex);
    // remove client from the channel
    int uIndex = channels[i].getUserIndex(clt->getNickname());
    if (uIndex != NOT_VALID) {
      channels[i].remove_user(uIndex, clt->getNickname());
      if (channels[i].getSize() == 0)
        channels.erase(channels.begin() + i);
    }
  }
  // erase his read_buffer
  std::map<int, std::string>::iterator it = read_buffer.find(clt->getFd());
  if (it != read_buffer.end())
    read_buffer.erase(it);
  // erase him from poll_fds vector
  for (size_t i = 0; i < poll_fds.size(); i++) {
    if (poll_fds[i].fd == clt->getFd())
      poll_fds.erase(poll_fds.begin() + i);
  }
  // erase him from clients vector
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i]->getFd() == clt->getFd())
      clients.erase(clients.begin() + i);
  }
  close(clt->getFd());
  delete clt;
}

void send_reply(int fd, std::string str) {
  send(fd, str.c_str(), str.length(), 0);
}

void server::HandleData(client *clt) {
  char buf[BUFFER_SIZE];
  ssize_t rn = recv(clt->getFd(), buf, BUFFER_SIZE - 1, 0);
  if (rn < 0)
    throw std::runtime_error("recv() failed");
  else if (rn == 0)
    deleteClientData(clt);
  else {
    buf[rn] = '\0';
    read_buffer[clt->getFd()].append(buf);
    execute_cmds(clt);
  }
}

void server::start() {
  while (true) {
    if (poll(&poll_fds[0], poll_fds.size(), -1) == NOT_VALID)
      throw std::runtime_error("poll() failed");
    for (size_t i = 0; i < poll_fds.size(); i++) {
      if ((poll_fds[i].revents & POLLIN)) {
        if (poll_fds[i].fd == s_socket)
          this->CreateClient();
        else
          this->HandleData(clients[i - 1]);
      }
    }
  }
}

void server::authenticate_cmds(std::string line, client &clt) {
  std::string reply;
  std::vector<std::string> command = split_line(line, " \t\r");
  if (command[0].compare("PASS") == 0)
    return check_password(command, clt);
  if (command[0].compare("NICK") == 0 && clt.authentication[0])
    return check_nickname(command, clt);
  else if (command[0].compare("NICK") == 0)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  if (command[0].compare("USER") == 0 && clt.authentication[0] &&
      clt.authentication[1])
    return check_username(command, clt, line);
  else if (command[0].compare("USER") == 0)
    return send_reply(clt.getFd(),
                      ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
}

int server::channel_cmds(std::string line, client *clt) {
  std::vector<std::string> command = split_line(line, " \t\r");
  if (command[0].compare("JOIN") == 0)
    do_join(command, clt);
  else if (command[0].compare("PRIVMSG") == 0)
    do_privmsg(command, *clt, line);
  else if (command[0].compare("TOPIC") == 0)
    do_topic(command, *clt, line);
  else if (command[0].compare("INVITE") == 0)
    do_invite(command, *clt);
  else if (command[0].compare("KICK") == 0)
    do_kick(command, *clt, line);
  else if (command[0].compare("MODE") == 0)
    do_mode(command, *clt);
  else if (command[0].compare("PING") == 0)
    send_pong(command, *clt);
  else if (command[0].compare("QUIT") == 0)
  {
    for (size_t j = 0; j < clients.size(); j++)
    {
      if (clients[j]->getFd() != clt->getFd())
        send_reply(clients[j]->getFd(), RPL_QUIT(clt->getNickname(), "left the server"));
    }
    deleteClientData(clt);
    return 1;
  }
  else if (command[0].compare("NICK") != 0 && command[0].compare("USER") != 0 &&
           command[0].compare("PASS") != 0)
    send_reply(clt->getFd(), ERR_UNKNOWNCOMMAND(clt->getNickname(), command[0]));
  return 0;
}

void server::execute_cmds(client *clt) {
  size_t pos;
  std::string line;
  while ((pos = read_buffer[clt->getFd()].find("\n")) != std::string::npos) {
    line = read_buffer[clt->getFd()].substr(0, pos);
    std::cout << YELLOW << "line to parse : " << RESET;
    std::cout << line << std::endl;
    if (!line.empty()) {
      authenticate_cmds(line, *clt);
      if (clt->authentication[0] && clt->authentication[1] && clt->authentication[2])
        if (channel_cmds(line, clt) == 1)
          break ;
    }
    read_buffer[clt->getFd()].erase(0, pos + 1);
  }
}

std::vector<client*> server::getClients() const { return this->clients; }
