#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/**
 * @brief send message to the joining member
 */
void	Server::sendMsg(Client& client, std::string& msg)
{
	if (send(client.getClientFd(), msg.c_str(), msg.size(), 0) < 0)
	{
		std::cout << "joinmsg: failed to send\n";
		return;
	}
	// std::cout << "msg sent: " << msg << std::endl;
}

/**
 * @brief send message to all member on channels and the sender itself
 */
void Server::broadcastChannelMsg(std::string& msg, Channel& channel)
{
	for (Client* user : channel.getUserList())
		this->sendMsg(*user, msg);
	//recheck does this send to the joining memeber itself
}

/**
 * @brief send message to all member on channels EXCEPT the sender itself
 */
void Server::broadcastChannelMsg(std::string& msg, Channel& channel, Client& client)
{
	for (Client* user : channel.getUserList())
	{
		if (user->getNick() != client.getNick())
			this->sendMsg(*user, msg);
	}
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES) and the channel creation timestamp. 
 */
void	Server::sendJoinSuccessMsg( Client& client, Channel& channel)
{
	std::string	user = client.makeUser();

	// send JOIN msg
	std::string joinMsg = user + " JOIN #" + channel.getChannelName() + " \r\n";
	this->sendMsg(client, joinMsg);
	this->sendTopic(client, channel);
	this->sendNameReply(client, channel);
	this->sendClientErr(RPL_CREATIONTIME, client, &channel, {});
	this->broadcastChannelMsg(joinMsg, channel, client);
}


/** @brief send topic or no topic */
void	Server::sendTopic(Client& client, Channel& channel)
{
	std::string	server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName();

	if (!channel.getTopic().empty())
	{
		this->sendClientErr(RPL_TOPIC, client, &channel, {});
		this->sendClientErr(RPL_TOPICWHOTIME, client, &channel, {});
	}
}

/** @brief send list of users in channel */
void	Server::sendNameReply(Client& client, Channel& channel)
{
	std::string	server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName();

	this->sendClientErr(RPL_NAMREPLY, client, &channel, {});
	this->sendClientErr(RPL_ENDOFNAMES, client, &channel, {});
}

void Server::sendKickMsg(std::string oper, std::string client, std::vector<std::string>& params, Channel& channel)
{
	//check if a reason for kicking exists
	for (auto it : params)
		std::cout << " / " << it;
	std::cout << std::endl;
	std::string reason;
	if (params[2].length() > 1) {
		for (int i = 2; i < params.size(); ++i) {
			reason += params[i];
			if (i + 1 != params.size())
				reason += " ";
		}
	}
	else
		reason = oper;
	std::cout << reason << std::endl;
	std::string msg =	":" + oper + "!" + oper + "@localhost"
						+ " KICK " + "#" + channel.getChannelName()
						+ " " + client + " " + reason + "\r\n";
	broadcastChannelMsg(msg, channel);
}

void Server::sendClientErr(int num, Client& client, Channel* channel, std::vector<std::string> otherArgs)
{
	std::string server = this->getServerName(),
				nick = client.getNick(),
				chanName, msg, arg;
	if (channel)
		chanName = channel->getChannelName();
	
	switch (num)
	{
	case ERR_BADCHANNELKEY:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+k)");
		break;

	case ERR_TOOMANYCHANNELS:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "You have joined too many channels");
		break;

	case ERR_UNKNOWNMODE:
	{
		if (otherArgs.size() == 1) 
		{
			arg = otherArgs[0]; 
			msg = makeNumericReply(server, num, nick, {arg}, "is unknown mode char to me");
		};
		break;
	}

	case ERR_CHANNELISFULL:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+l)");
		break;

	case ERR_INVITEONLYCHAN:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+i)");
		break;
	
	case ERR_NOTONCHANNEL:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "You're not on that channel");
		break;
	
	case ERR_CHANOPRIVSNEEDED:
		msg = makeNumericReply(server, num,	nick, {"#" + chanName}, "You're not channel operator");
		break;

	case ERR_USERNOTINCHANNEL:
	{
		if (otherArgs.size() == 1) {arg = otherArgs[0]; };
		msg = makeNumericReply(server, num, nick, {arg,"#" + chanName}, "They aren't on that channel");
		break ;
	}

	case ERR_NOSUCHNICK:
	{
		if (otherArgs.size() == 1) {arg = otherArgs[0]; };
		msg = makeNumericReply(server, num, nick, {arg}, "No such nick/channel");
		break ;
	}

	case ERR_NOSUCHCHANNEL:
	{
		if (otherArgs.size() == 1) 
		{
			chanName = otherArgs[0]; 
			msg = makeNumericReply(server, num,	nick, {chanName}, "No such channel");
		};
		break;
	}

	case ERR_USERONCHANNEL:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num,	nick, {arg ,"#" + chanName}, "is already on channel");
		};
		break;
	}

	case ERR_CANNOTSENDTOCHAN:
		msg = makeNumericReply(server, num,	nick, {"#" + chanName}, "Cannot send to channel");
		break;
	
	case ERR_NORECIPIENT:
		msg = makeNumericReply(server, num,	nick, {}, "No recipient given");
		break;
	
	case ERR_NOTEXTTOSEND:
		msg = makeNumericReply(server, num,	nick, {}, "No text to send");
		break;

	
	//RPL	
	case RPL_NOTOPIC:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "No topic is set");
		break;
	
	case RPL_TOPIC:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, channel->getTopic());
		break;

	case RPL_TOPICWHOTIME:
		msg = makeNumericReply(server, num, nick, {"#" + chanName, 
			channel->getTopicSetter()->getNick(), channel->getTopicTimestamp()}, "");
		break;

	case RPL_NAMREPLY:
		msg = makeNumericReply(server, num, nick,  {"=", "#"+ chanName}, 
			channel->printUser());
		break;

	case RPL_ENDOFNAMES:
		msg = makeNumericReply(server, num, nick, {"#" + chanName},	"End of /NAMES list.");
		break;

	case RPL_INVITING:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num,	nick, {arg ,"#" + chanName}, "");
		};
		break;
	}

	case RPL_CREATIONTIME:
		msg = makeNumericReply(server, num, nick, {"#" + chanName, 
			channel->getChannelCreationTimestamp()}, "");
		break;

	case RPL_CHANNELMODEIS:
	{
		if (otherArgs.size() == 2)
		{
			std::string modeStr = otherArgs[0];
			std::string modeArgs = otherArgs[1];
			msg = makeNumericReply(server, num, nick, {"#" + chanName, modeStr, modeArgs}, 
				"");
		}
		break;
	}

	// duplicate

	case 461:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = ERR_NEEDMOREPARAMS(server, nick, arg);
			// msg = makeNumericReply(server, num, nick, otherArgs, "Not enough parameters");
		};
		break;	
	}
	
	default:
		break;
	}
	this->sendMsg(client, msg);
}
