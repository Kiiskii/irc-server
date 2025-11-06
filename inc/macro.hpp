#pragma once

#define MAX_CHANNELS_PER_CLIENT 5 //recheck CHANLIMIT IN RP_SUPPORT

#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332"
#define RPL_NAMREPLY "353"
#define RPL_TOPICWHOTIME "333"
#define ERR_TOOMANYCHANNELS "405"
#define ERR_NOTONCHANNEL "442"
#define ERR_BADCHANNELKEY "475"

#define RPL_WELCOME(servername, nickname) ":" + servername + " 001 " + nickname + " :Welcome to the " + servername + " Network, " + nickname + "\r\n"

#define NEW_NICK(oldnick, user, host, newnick) ":" + oldnick + "!" + user + "@" + host + " NICK " + newnick + "\r\n"
#define ERR_NONICKNAMEGIVEN(servername) ":" + servername + "431 :No nickname given\r\n"
 // "<client> :Welcome to the <networkname> Network, <nick>[!<user>@<host>]"


    // "<client> :No nickname given" (431)
