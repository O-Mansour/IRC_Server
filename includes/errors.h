#pragma once

//=========Errors===============
// Welcome

#define RPL_WELCOME(client)                                                    \
  (":localhost.irc.com 001 " + client +                                        \
   " :Welcome to the Internet Relay Network \r\n")
#define RPL_YOURHOST()                                                         \
  (":localhost.irc.com 002 IRSSI :Your host is localhost, running version "    \
   "1.0\r\n")
#define RPL_CREATED()                                                          \
  (":localhost.irc.com 003 IRSSI :This server was created " +                  \
   print_time_welcome() + "\r\n")
#define RPL_MYINFO()                                                           \
  (":localhost.irc.com 004 IRSSI localhost 1.0 CHANTYPES=#\r\n")
#define RPL_ISUPPORT() (":localhost.irc.com 422 IRSSI :MOTD is missing\r\n")

// PASS :
#define ERR_NEEDMOREPARAMS()                                                   \
  (RED "localhost: Not enough parameters" RESET "\r\n")
#define ERR_ALREADYREGISTERED(client)                                          \
  (RED "localhost: " + client + " :You may not reregister" RESET "\r\n")
#define ERR_PASSWDMISMATCH() (RED "localhost: Password incorrect" RESET "\r\n")
// NICK :
#define ERR_NONICKNAMEGIVEN(client)                                            \
  (RED "localhost: No nickname given" RESET "\r\n")
#define ERR_ERRONEUSNICKNAME() (RED "localhost: Invalid character" RESET "\r\n")
#define ERR_NICKNAMEINUSE(client, nick)                                        \
  (RED "localhost: " + client + " This \"" + nick +                            \
   "\" Nickname is already in use" RESET "\r\n")
#define ERR_NICKNAMECHANGE(client, nick)                                       \
  (GREEN "localhost: " + client + " nickname is now " + nick + RESET " \r\n")
#define ERR_NICKNAMEFIRST()                                                    \
  (RED "localhost: Enter a nickname first" RESET "\r\n")
#define ERR_NICKNAMEINVALID(client)                                            \
  (RED "localhost: " + client + " Reserved for the bot" RESET "\r\n")

// USER
#define ERR_USERFORMAT()                                                       \
  (UNDERLINE "localhost: Please set the second parameter with <0> and the "    \
             "third with <*>" RESET "\r\n")
#define ERR_USERSYNTAX()                                                       \
  (RED "localhost: Please use this syntax : USER <username> 0 * "              \
       ":<realname>" RESET "\r\n")

// PRIVMSG

#define ERR_MSGSYNTAX()                                                        \
  (RED "localhost: Please use this syntax : PRIVMSG <channel/user> "           \
       ":<your_msg_here>" RESET "\r\n")
#define ERR_NOSUCHNICK() (RED "No such nick" RESET "\r\n")
#define ERR_NOSUCHCHANNEL() (RED "No such channel" RESET "\r\n")
#define ERR_USERONCHANNEL(client, channel)(RED + client + " is already on channel :#" + channel + RESET "\r\n")
#define ERR_CHANOPRIVSNEEDED(client, channel)(RED + client + " " + channel + " :You're not channel operator" RESET "\r\n")
#define RPL_INVITING(client,nick,channel) (":localhost 341 " + client + " " + nick + " #" + channel)
#define ERR_NOTONCHANNEL(client,channel)(":localhost 442 " + client + " #" + channel + " :You're not on that channel")
