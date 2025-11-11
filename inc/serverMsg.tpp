#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "macro.hpp"


template <typename T>
std::string Channel::channelMessage(channelMsg msg, T& extraInfo)
{
	std::string returnMsg;
	std::string chanop = ":" + extraInfo.client->getNick() + "!" 
	+ extraInfo.client->getUserName() + "@" + extraInfo.client->getHostName();

	switch (msg)
	{
	case JOIN_OK:
		returnMsg = chanop + " JOIN #" + this->getChannelName() 
		+" " + RPL_TOPIC + " \r\n";
		break;

	case TOO_MANY_CHANNELS: 	
		returnMsg = ":" + extraInfo.client->getServerName() + " " + ERR_TOOMANYCHANNELS 
		+ " " + extraInfo.client->getNick() + " " + this->getChannelName() 
		+" :You have joined too many channels" + " \r\n";
		break;

	case SET_MODE_OK: 	
		returnMsg = chanop + " MODE #" + this->getChannelName() + " " + extraInfo.addMode
		+ " " + extraInfo.mode + " \r\n";
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
	
	case CHANGE_TOPIC_MSG:
		returnMsg = chanop + " TOPIC #" + this->getChannelName() +" :" 
		+ this->getTopic() + "\r\n";
		break;

	// case NAME_LIST_MSG:
	// 	returnMsg = ":" + currentClient._serverName + RPL_NAMREPLY  + "!" + 
	// 	currentClient._clientNick + "=#" + currentClient._atChannel->getChannelName() 
	// 	+ ":" + currentClient._atChannel->getUserList() +" \r\n";

	default:
		break;
	}
	std::cout << "return mes: " << returnMsg << std::endl;
	return returnMsg;
}