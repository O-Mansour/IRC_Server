#pragma once

//=========Errors===============
// Welcome

#define RPL_WELCOME(client, nickname)                                                    \
  (":localhost 001 " + client + " :Welcome to the Internet Relay Network, " + nickname + "\r\n")
#define RPL_YOURHOST(client)                                                         \
  (":localhost 002 " + client + " :Your host is FT_IRC, running version 1.0\r\n")
#define RPL_CREATED(client)                                                          \
  (":localhost 003 " + client + " :This server was created " +                  \
   print_time_welcome() + "\r\n")
#define RPL_MYINFO(client)                                                           \
  (":localhost 004 " + client + " FT_IRC 1.0\r\n")
#define RPL_ISUPPORT(client) (":localhost 005 + " + client + " CHANTYPES=# CHANNELLEN=50 NICKLEN=15 TOPICLEN=400 :are supported by this server\r\n")

// PASS :
#define ERR_NEEDMOREPARAMS(client,command)                                                   \
  (":localhost 461 " + client + " " + command + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTERED(client) (":localhost 462 " + client + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(client) (":localhost 464 " + client + " :Password incorrect\r\n")
// NICK :
#define ERR_NONICKNAMEGIVEN(client) (":localhost 431 " + client + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(client, nickname) (":localhost 432 " + client + " " + nickname + " :Erroneus nickname\r\n")
#define ERR_NICKNAMEINUSE(client, nickname) (":localhost 433 " + client + " " + nickname + " :Nickname is already in use\r\n")
#define RPL_NICK(oclient, nclient) (":" + oclient + "!@localhost NICK " +  nclient + "\r\n")

// USER
#define ERR_USERFORMAT()                                                       \
  (":localhost Please set the second parameter with <0> and the third with <*>\r\n")
#define ERR_USERSYNTAX()                                                       \
  (":localhost Please use this syntax : USER <username> 0 * "              \
       ":<realname>\r\n")

// PRIVMSG
# define RPL_PRIVMSG(nick, target, message) (":" + nick + "!@localhost PRIVMSG " + target + " :" + message + "\r\n")

#define ERR_NOSUCHNICK(client,nickname) (":localhost 401 " + client + " " + nickname + " No such nick/channel\r\n")
#define ERR_NOSUCHCHANNEL(client, channel) (":localhost 403 " + client + " #" + channel + " :No such channel\r\n")
#define ERR_USERONCHANNEL(client,nickname, channel)(":localhost 443 "+ client + " " + nickname + " #" + channel + " :is already on channel\r\n")
#define ERR_CHANOPRIVSNEEDED(client, channel)(":localhost 482 " + client + " #" + channel + " :You're not channel operator\r\n")
#define RPL_INVITING(client,nick,channel) (":localhost 341 " + client + " " + nick + " #" + channel + "\r\n")
#define ERR_NOTONCHANNEL(client,channel)(":localhost 442 " + client + " #" + channel + " :You're not on that channel\r\n")
# define ERR_USERNOTINCHANNEL(client, nickname, channel) (":localhost 441 " + client + " " + nickname + " #" + channel + " :They aren't on that channel\r\n")
//JOIN :
# define ERR_BADCHANNELKEY(nick, channel) (":localhost 475 " + nick + " #" + channel + " :Cannot join channel (+k)\r\n")
# define RPL_JOIN(nick, channel) (":" + nick + "@localhost JOIN :#" + channel + "\r\n")
# define RPL_NAMREPLY(nick, channel, nicks_list) (":localhost 353 " + nick + " = #" + channel + " :" + nicks_list + "\r\n")
# define RPL_ENDOFNAMES(nick, channel) (":localhost 366 " + nick + " #" + channel + " :End of /NAMES list\r\n")
# define ERR_INVITEONLYCHAN(nick, channel) (":localhost 473 " + nick + " #" + channel + " :Cannot join channel (+i)\r\n")
# define ERR_CHANNELISFULL(nick, channel) (":localhost 471 " + nick + " #" + channel + " :Cannot join channel (+l)\r\n")

// TOPIC
# define RPL_TOPIC(nick, channel, topic) (":localhost 332 " + nick + " #" + channel + " :" + topic + "\r\n")
# define RPL_NOTOPIC(nick, channel) (":localhost 331 " + nick + " #" + channel + " :No topic is set\r\n")
# define TOPIC_TO_ALL(nick, channel, topic) (":" + nick + "@localhost TOPIC #" + channel + " :" + topic + "\r\n")

// MODE
# define RPL_MODE(channel, mode, user) (":localhost MODE #" + channel + " " + mode + " " + user + "\r\n")
# define ERR_UMODEUNKNOWNFLAG(nick) (":localhost 501 " + nick + " :Unknown MODE flag\r\n")
# define RPL_CHANNELMODEIS(nick, channel, modes) (":localhost 324 " + nick + " #" + channel + " +" + modes + "\r\n")

// QUIT
# define RPL_QUIT(nick, reason) (":" + nick + "@localhost QUIT :Quit: " + reason + "\r\n")

//UNKOWN
# define ERR_UNKNOWNCOMMAND(nick, command) (":localhost 421 " + nick + " " + command + " :Unknown command\r\n")
# define RPL_PART(client, channel, reason) (":" + client + "@localhost PART #" + channel + " " + reason + "\r\n")

//kick
// #define RPL_KICK(client,kicked,channel,reason)(":" + client + "!~h@localhost Kicked " + kicked + " from #" + channel + " [" + reason + "]\r\n")
#define RPL_KICK(client, channel, kicked, reason) (":" + client + "@localhost KICK #" + channel + " " + kicked + " :" + reason + "\n")
