#pragma once

#define MSG_SIZE 512
#define NICKLEN 15
#define USERLEN 15
#define CHANMODES "itkol"
#define MAX_CHANNELS_PER_CLIENT 5 //recheck CHANLIMIT IN RP_SUPPORT

#define L_MODE 'l'
#define I_MODE 'i'
#define T_MODE 't'
#define K_MODE 'k'
#define O_MODE 'o'

/* REPLIES */
#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_NAMREPLY 353
#define RPL_TOPICWHOTIME 333
#define RPL_ENDOFNAMES 366
#define RPL_INVITING 341

/* ERRORS */
#define ERR_NOSUCHCHANNEL 403
#define ERR_TOOMANYCHANNELS 405
#define ERR_UNKNOWNMODE 472
#define ERR_BADCHANNELKEY 475
#define ERR_NOTONCHANNEL 442
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_CHANNELISFULL 471
#define ERR_INVITEONLYCHAN 473
#define ERR_NOSUCHNICK 401
#define ERR_USERONCHANNEL 443


// #define ERR_NEEDMOREPARAMS 461 //duplicate, commnt out if needed

//#define ERR_NEEDMOREPARAMS 461 //duplicate, commnt out if needed
// RPL_MOTDSTART (375) 
//   "<client> :- <server> Message of the day - "
// RPL_MOTD (372) 
//   "<client> :<line of the motd>"
// RPL_ENDOFMOTD (376) 
//   "<client> :End of /MOTD command."

#define RPL_WELCOME(servername, nickname) ":" + servername + " 001 " + nickname + " :Welcome to the " + servername + " Network, " + nickname + "\r\n"
#define RPL_YOURHOST(servername, nickname, version) ":" + servername + " 002 " + nickname + " :Your host is " + servername + ", running version " + version + "\r\n"
#define RPL_CREATED(servername, nickname, datetime) ":" + servername + " 003 " + nickname + " :This server was created " + datetime + "\r\n"
#define RPL_MYINFO(servername, nickname, version, umodes, cmodes) ":" + servername + " 004 " + nickname + " " + servername + " " + version + " " + umodes + " " + cmodes + "\r\n"
#define RPL_ISUPPORT(servername, nickname, info) ":" + servername + " 005 " + nickname + " " + info + ":are supported by this server\r\n"

#define RPL_PONG(token) "PONG " + token + "\r\n"
#define NEW_NICK(oldnick, user, host, newnick) ":" + oldnick + "!" + user + "@" + host + " NICK " + newnick + "\r\n"
#define ERR_NONICKNAMEGIVEN(servername, nickname) ":" + servername + " 431 " + nickname + " :No nickname given\r\n"
#define ERR_ERRONEUSNICKNAME(servername, nickname, badnick) ":" + servername + " 432 " + nickname + " " + badnick + " :Erroneus nickname\r\n"
#define ERR_NICKNAMEINUSE(servername, nickname, badnick) ":" + servername + " 433 " + nickname + " " + badnick + " :Nickname is already in use\r\n"
#define ERR_NEEDMOREPARAMS(servername, nickname, command) ":" + servername + " 461 " + nickname + " " + command + " :Not enough parameters\r\n"
#define ERR_PASSWDMISMATCH(servername, nickname) ":" + servername + " 464 " + nickname + " :Password incorrect\r\n"
#define ERR_ALREADYREGISTERED(servername, nickname) ":" + servername + " 462 " + nickname + " :You may not register\r\n"

//should this also have the servername and nickname?
#define ERR_GENERIC(servername, nickname, reason) ":" + servername + " " + nickname + " :" + reason + "\r\n"
//..this is example
// The ERR_NEEDMOREPARAMS response should follow the format:

// text
// :<server> 461 <client> <command> :Not enough parameters
// If the client is not yet registered 
//(example: during the PASS command before NICK or USER is set), use * or a connection identifier in place of <client>.​




