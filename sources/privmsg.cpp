#include "Server.hpp"
#include "replies.hpp"

void server::do_privmsg(std::vector<std::string> &command, client &clt,
                        std::string line) {
  if (command.size() < 3)
    send_reply(clt.getFd(), ERR_NEEDMOREPARAMS(clt.getNickname(), command[0]));
  else {
    size_t i;
    if (command[1].at(0) == '#') {
      command[1].erase(0, 1);
      line = extract_param(command, line, 2);
      for (i = 0; i < channels.size(); i++) {
        if (channels[i].getName().compare(command[1]) == 0) {
					if (channels[i].getUserIndex(clt.getNickname()) == NOT_VALID)
						return send_reply(clt.getFd(), ERR_CANNOTSENDTOCHAN(clt.getNickname(), command[1]));
          channels[i].c_privmsg(clt, line);
          // TODO: if the bot joined a channel. check forward messages to bot
          if (channels[i].getIsBotJoined()) {
            this->bot.setMessage(line);
            std::string msg = this->bot.getResponse(channels[i].getName());
            if (msg.compare("-1") != 0 && msg.find("-2") == std::string::npos) {
              channels[i].msgToAllMemebers(msg);
            }
            if (msg.find("-2") != std::string::npos) {
              channels[i].kick_user_msg(
                  RPL_KICK(this->bot.getNickname(), channels[i].getName(),
                           clt.getNickname(), " Dont use bad words"));
              int res = channels[i].getUserIndex(clt.getNickname());
              if (res != -1)
                channels[i].remove_user(res, clt.getNickname());
            }
            return;
          }
          break;
        }
      }
      if (i == channels.size())
        send_reply(clt.getFd(),
                   ERR_NOSUCHCHANNEL(clt.getNickname(), command[1]));
    } else if (line.find("\001DCC SEND ") != std::string::npos ||
               line.find("\001DCC CHAT ") != std::string::npos) {
      // handle the dcc connection and send responce to the client

      Dcc dccHandler(clt);
      dccHandler.setData(line, this->getClients());
      send_reply(dccHandler.getCltFd(), dccHandler.getClientResponce());
    } else {
      line = extract_param(command, line, 2);
      // checking for bot, and forward message to bot
      if (command[1].compare(this->bot.getNickname()) == 0) {
        this->bot.setMessage(line);
      } else {
        for (i = 0; i < clients.size(); i++) {
          if (command[1].compare(clients[i]->getNickname()) == 0) {
            send_reply(
                clients[i]->getFd(),
                RPL_PRIVMSG(clt.getNickname(), clients[i]->getNickname(), line));
            break;
          }
        }
        if (i == clients.size())
          send_reply(clt.getFd(),
                     ERR_NOSUCHNICK(clt.getNickname(), command[1]));
      }
    }
  }
}

