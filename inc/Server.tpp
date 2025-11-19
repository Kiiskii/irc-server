#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"



template <typename ...args>
void Server::channelMessage(channelMsg msg, args ...moreArgs)
{
	// this part is mainly for mode, considering split up or ??
	auto		tupleArgs = std::make_tuple(moreArgs...);
	constexpr size_t		nArgs = sizeof...(moreArgs);
	std::cout << "Tuple size: " << nArgs << std::endl;

	std::string	user, mode, params, modeStr;
	Client*		client;
	Channel*	channel;
	if constexpr (nArgs > 0)
		client = std::get<0>(tupleArgs);

	if constexpr (nArgs > 1)
		channel = std::get<1>(tupleArgs);

	if constexpr (nArgs > 2)
	{
		mode = std::get<2>(tupleArgs);
		modeStr += mode; 
	}

	if constexpr (nArgs > 3)
	{
		params = std::get<3>(tupleArgs);
		modeStr += " " + params;
	}

	std::string server = client->_myServer.getServerName(),
				nick = client->getNick(),
				chanName = channel->getChannelName();

	switch (msg)
	{
	case JOIN_OK:
		this->sendJoinSuccessMsg(*client, *channel);
		break;
	
	case ALREADY_ON_CHAN:
		this->sendTopicAndNames(*client, *channel);
		break;

	case CHANGE_TOPIC_MSG:
	{
		std::string	returnMsg = client->makeUser() + " TOPIC #" + 
			channel->getChannelName() +" :" + channel->getTopic() + "\r\n";
		this->broadcastChannelMsg(returnMsg, *channel);
		break;
	}

	case SET_MODE_OK:
	{
		std::string	modeMsg = client->makeUser() + " MODE #" + 
			channel->getChannelName() + " " + modeStr + " \r\n";
		this->broadcastChannelMsg(modeMsg, *channel);
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
