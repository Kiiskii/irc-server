#include "Channel.hpp"
#include "Client.hpp"


template <typename ...args>
std::string Channel::channelMessage(channelMsg msg, args ...moreArgs)
{
	auto		tupleArgs = std::make_tuple(moreArgs...);
	std::cout << "Tuple size: " << sizeof...(moreArgs)<< std::endl;
    // if constexpr (sizeof...(moreArgs) > 0) {
    //     std::cout << "First element (Client*): " << std::get<0>(tupleArgs)->getNick() << std::endl;
    // }

    // if constexpr (sizeof...(moreArgs) > 1) {
    //     std::cout << "Second element (mode): " << std::get<1>(tupleArgs) << std::endl;
    // }

    // if constexpr (sizeof...(moreArgs) > 2) {
    //     std::cout << "Third element (params): " << std::get<2>(tupleArgs) << std::endl;
    // }

	Client*		client = std::get<0>(tupleArgs);
	std::string	returnMsg;
	std::string	chanop = ":" + client->getNick() + "!" 
		+ client->getUserName() + "@" + client->getHostName();
	std::string mode, params;
	std::string modeStr = "";

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
		if constexpr (sizeof...(moreArgs) > 1)
		{
			mode = std::get<1>(tupleArgs);
			modeStr += mode; 
		}
		else 
			std::cout << "no 2nd element\n";
		if constexpr (sizeof...(moreArgs) > 2) 
		{
			params = std::get<2>(tupleArgs);
			modeStr += " " + params;
		}
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
	std::cout << "return mes: " << returnMsg << std::endl;
	return returnMsg;
}