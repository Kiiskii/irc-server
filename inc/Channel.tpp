#include "Channel.hpp"
#include "Client.hpp"


template <typename ...args>
std::string Channel::channelMessage(channelMsg msg, args ...moreArgs)
{
	auto		tupleArgs = std::make_tuple(moreArgs...);
	constexpr size_t		nArgs = sizeof...(moreArgs);
	std::cout << "Tuple size: " << nArgs << std::endl;

	std::string	returnMsg, chanop, mode, params, modeStr;
	Client*		client;
	if constexpr (nArgs > 0)
	{
		client = std::get<0>(tupleArgs);
		chanop = ":" + client->getNick() + "!" 
			+ client->getUserName() + "@" + client->getHostName();
	}
	// std::string mode, params;
	// std::string modeStr = "";
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
		returnMsg = chanop + " JOIN #" + this->getChannelName() 
			+" " + RPL_TOPIC + " \r\n";
		break;

	case TOO_MANY_CHANNELS: 	
		returnMsg = ":" + client->getServerName() + " " + ERR_TOOMANYCHANNELS 
		+ " " + client->getNick() + " " + this->getChannelName() 
		+" :You have joined too many channels" + " \r\n";
		break;

	case SET_MODE_OK:
		returnMsg = chanop + " MODE #" + this->getChannelName() + " " + modeStr + " \r\n";
		break;
		
	case CHANGE_TOPIC_MSG:
		returnMsg = chanop + " TOPIC #" + this->getChannelName() +" :" 
		+ this->getTopic() + "\r\n";
		break;


	// not test this yet, need key
	// case BAD_CHANNEL_KEY: 	
	// 	returnMsg = ":" + extraInfo->client->getServerName() + " " + ERR_TOOMANYCHANNELS 
	// 	+ " " + extraInfo->client->getNick() + " " + this->getChannelName() 
	// 	+" :Cannot join channel" + " \r\n";
	// 	break;
	
	// case NO_TOPIC_MSG:
	// 	returnMsg = ":" + currentClient->_serverName + " " + RPL_NOTOPIC + " " + 
	// 	currentClient->_clientNick + " #" + this->getChannelName() 
	// 	+ " :No topic is set\r\n";
	// 	break;
	
	// case CHANNEL_TOPIC_MSG:
	// 	returnMsg = ":" + currentClient->_serverName + " " + RPL_TOPIC + " " + 
	// 	currentClient->_clientNick + " #" + this->getChannelName() 
	// 	+ " :" + this->getTopic() +" \r\n";
	// 	break;
	
	// case WHO_CHANGE_TOPIC:
	// 	returnMsg = ":" + currentClient->_serverName + " " + RPL_TOPICWHOTIME + " " + 
	// 	currentClient->_clientNick + " #" + this->getChannelName() 
	// 	+ " :" + this->getTopic() +" \r\n";
	// 	break;
	
	
	// case NAME_LIST_MSG:
	// 	returnMsg = ":" + currentClient._serverName + RPL_NAMREPLY  + "!" + 
	// 	currentClient._clientNick + "=#" + currentClient._atChannel->getChannelName() 
	// 	+ ":" + currentClient._atChannel->getUserList() +" \r\n";

	default:
		returnMsg = "NO MESSAGE";
		break;
	}
	// std::cout << "msg sent by server: " << returnMsg << std::endl;
	return returnMsg;
}