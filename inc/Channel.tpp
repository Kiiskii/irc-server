#include "Channel.hpp"
#include "Client.hpp"



/* The most common form of reply is the numeric reply, used for both errors and normal replies. Distinct from a normal message, a numeric reply MUST contain a <source> and use a three-digit numeric as the command. A numeric reply SHOULD contain the target of the reply as the first parameter of the message. A numeric reply is not allowed to originate from a client */
// : channel names, usernames, modes, numbers -> param
// Everything after : is considered the trailing field
static std::string makeNumericReply(std::string prefix, int code, std::string target, std::vector<std::string> params, std::string trailing)
{
	std::string p, s;
	for (auto param : params)
		p += param + " ";
	s = ":" + prefix + " " + std::to_string(code) + " " + target 
		+ (p.empty() ? "" : " " + p)
		+ (trailing.empty() ? "" : ":" + trailing)
		+ "\r\n";
	return s;
}

template <typename ...args>
void Channel::channelMessage(channelMsg msg, args ...moreArgs)
{
	// this part is mainly for mode, considering split up or ??
	auto		tupleArgs = std::make_tuple(moreArgs...);
	constexpr size_t		nArgs = sizeof...(moreArgs);
	std::cout << "Tuple size: " << nArgs << std::endl;

	std::string	user, mode, params, modeStr;
	Client*		client;
	if constexpr (nArgs > 0)
	{
		client = std::get<0>(tupleArgs);
	}
	if constexpr (nArgs > 1)
	{
		mode = std::get<1>(tupleArgs);
		modeStr += mode; 
	}

	if constexpr (nArgs > 2)
	{
		params = std::get<2>(tupleArgs);
		modeStr += " " + params;
	}

	switch (msg)
	{
	case JOIN_OK:
		this->sendJoinSuccessMsg(client);
		break;
	
	case ALREADY_ON_CHAN:
		this->sendTopicAndNames(client);
		break;

	case TOO_MANY_CHANNELS:
	{
		std::string manyChannelsMsg = makeNumericReply(client->getServerName(), 
		ERR_TOOMANYCHANNELS, client->getNick(), {"#" + this->getChannelName()}, "You have joined too many channels");
		this->sendMsg(client, manyChannelsMsg);
		break;
	}
	
	case BAD_CHANNEL_KEY: //not test this yet
	{
		std::string badKeyMsg = makeNumericReply(client->getServerName(), ERR_BADCHANNELKEY, client->getNick(), {"#" + this->getChannelName()}, "Cannot join channel (+k)");
		this->sendMsg(client, badKeyMsg);
		break;
	}

	
	// case SET_MODE_OK:
	// 	returnMsg = chanop + " MODE #" + this->getChannelName() + " " + modeStr + " \r\n";
	// 	break;
		
	// case CHANGE_TOPIC_MSG:
	// 	returnMsg = chanop + " TOPIC #" + this->getChannelName() +" :" 
	// 	+ this->getTopic() + "\r\n";
	// 	break;

	// // NOT TEST YET
	


	default:
		std::string s = "NO MESSAGE";
		this->sendMsg(client, s);
		break;
	}
	// std::cout << "msg sent by server: " << returnMsg << std::endl;
	
}