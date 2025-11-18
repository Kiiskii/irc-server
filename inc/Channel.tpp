#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"


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

	std::string server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName();
	switch (msg)
	{
	case JOIN_OK:
		this->sendJoinSuccessMsg(client);
		break;
	
	case ALREADY_ON_CHAN:
		this->sendTopicAndNames(client);
		break;

	case CHANGE_TOPIC_MSG:
	{
		std::string	returnMsg = client->makeUser() + " TOPIC #" + 
			this->getChannelName() +" :" + this->getTopic() + "\r\n";
		this->broadcastChannelMsg(client, returnMsg);
		break;
	}

	case SET_MODE_OK:
	{
		std::string	modeMsg = client->makeUser() + " MODE #" + 
			this->getChannelName() + " " + modeStr + " \r\n";
		// std::cout << "mode msg: " << modeMsg << std::endl;
		this->broadcastChannelMsg(client, modeMsg);
		break;
	}

	default:
	{
		// std::string s = "NO MESSAGE";
		// this->sendMsg(client, s);
		break;
	}
	}
}
