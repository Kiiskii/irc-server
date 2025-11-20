#include "Server.hpp"

/**
 * @brief send message to the joining member, does it go to all members??
 */
void	Server::sendMsg(Client& client, std::string& msg)
{
	if (send(client.getClientFd(), msg.c_str(), msg.size(), 0) < 0)
	{
		std::cout << "joinmsg: failed to send\n";
		return;
	}
	std::cout << "msg sent: " << msg << std::endl;
}

/**
 * @brief send message to all member on channels and the joining member itself
 */
void Server::broadcastChannelMsg(std::string& msg, Channel& channel)
{
	std::cout << "broadcast msg: " << std::endl;
	for (Client* user : channel.getUserList())
		this->sendMsg(*user, msg);
	//recheck does this send to the joining memeber itself
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES). 
 */
void	Server::sendJoinSuccessMsg( Client& client, Channel& channel)
{
	std::string	user = client.makeUser();

	// send JOIN msg
	std::string joinMsg = user + " JOIN #" + channel.getChannelName() 
			+ " " + std::to_string(RPL_TOPIC) + " \r\n";
	this->sendMsg(client, joinMsg);
	this->sendTopicAndNames(client, channel);
}

void	Server::sendNoTopic(Client& client, Channel& channel)
{
	this->sendClientErr(RPL_NOTOPIC, client, channel, {});
}

void	Server::sendTopic(Client& client, Channel& channel)
{
	this->sendClientErr(RPL_TOPIC, client, channel, {});
	this->sendClientErr(RPL_TOPICWHOTIME, client, channel, {});
}

void	Server::sendTopicAndNames(Client& client, Channel& channel)
{
	std::string	server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName();

	// send topic / no_topic
	if (channel.getTopic().empty())
		this->sendNoTopic(client, channel);
	else
		this->sendTopic(client, channel);

	// send name list and end of list
	std::string nameReplyMsg = makeNumericReply(server, RPL_NAMREPLY, nick,  {"=", "#"+ chanName}, channel.printUser() );
	
	this->sendMsg(client, nameReplyMsg);
	// std::cout << "SENT NAMEPLY\n";
	
	std::string endOfNamesMsg = makeNumericReply(server, RPL_ENDOFNAMES, 
		nick, {"#" + chanName},	"End of /NAMES list.");
	this->sendMsg(client, endOfNamesMsg);
	// std::cout << "SENT ENDOFNAMES\n";
}


void Server::sendClientErr(int num, Client& client, Channel& channel, std::vector<std::string> otherArgs)
{
	std::string server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName(),
				msg, arg;

	if (!otherArgs.empty())
	{
		if (otherArgs.size() == 1) {arg = otherArgs[0]; };
		// if (otherArgs.size() == 1) {command = otherArgs[0]; };
	}
	
	switch (num)
	{
	case ERR_BADCHANNELKEY:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+k)");
		break;

	case ERR_TOOMANYCHANNELS:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "You have joined too many channels");
		break;

	case ERR_UNKNOWNMODE:
		msg = makeNumericReply(server, num, nick, {arg}, "is unknown mode char to me");
		break;

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

	case ERR_NOSUCHCHANNEL: //cannot dereference the nullptr, so this segfault. what now??
		msg = makeNumericReply(server, num,	nick, {"#" + chanName}, "No such channel");
		break;

	

	//RPL	
	case RPL_NOTOPIC:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "No topic is set");
		break;
	
	case RPL_TOPIC:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, channel.getTopic());
		break;

	case RPL_TOPICWHOTIME:
		msg = makeNumericReply(server, num, nick, {"#" + chanName, channel.getTopicSetter()->getNick(), std::to_string(channel.getTopicTimestamp())}, "");
		break;

	case 461:
		msg = ERR_NEEDMOREPARAMS(server, nick, "MODE"); // need fix
		// msg = makeNumericReply(server, num, nick, otherArgs, "Not enough parameters");
		break;	
	
	
	
	default:
		break;
	}
	this->sendMsg(client, msg);
}
