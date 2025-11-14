#pragma once

#define MAX_CHANNELS_PER_CLIENT 5 //recheck CHANLIMIT IN RP_SUPPORT

// #define RPL_NOTOPIC(server, nick, channelName) ":" + server + " 331 " + nick + " #" + channelName + " :No topic is set\r\n";
#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_NAMREPLY 353
#define RPL_TOPICWHOTIME 333
#define RPL_ENDOFNAMES 366
#define ERR_TOOMANYCHANNELS "405"
#define ERR_NOTONCHANNEL "442"
#define ERR_BADCHANNELKEY "475"

#define RPL_WELCOME(servername, nickname) ":" + servername + " 001 " + nickname + " :Welcome to the " + servername + " Network, " + nickname + "\r\n"

#define NEW_NICK(oldnick, user, host, newnick) ":" + oldnick + "!" + user + "@" + host + " NICK " + newnick + "\r\n"
#define ERR_NONICKNAMEGIVEN(servername) ":" + servername + "431 :No nickname given\r\n"
#define ERR_ERRONEUSNICKNAME(servername, nickname) ":" + servername + " 432 " + nickname + " :Erroneus nickname\r\n"
#define ERR_NICKNAMEINUSE(servername, nickname) ":" + servername + " 433 " + nickname + " :Nickname is already in use\r\n"

#define ERR_NEEDMOREPARAMS(servername, command) ":" + servername + " 461 * " + command + " :Not enough parameters\r\n"
#define ERR_PASSWDMISMATCH(servername) ":" + servername + " 464 * : Password incorrect\r\n"
#define ERR_ALREADYREGISTERED(servername, nickname) ":" + servername + " 462 " + nickname + " :You may not register\r\n"

//..this is example
// The ERR_NEEDMOREPARAMS response should follow the format:

// text
// :<server> 461 <client> <command> :Not enough parameters
// If the client is not yet registered 
//(example: during the PASS command before NICK or USER is set), use * or a connection identifier in place of <client>.​
