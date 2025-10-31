#include "Channel.hpp"

/// mode is set to +nt for now: n - no external message, t - topic restriction
Channel::Channel() : _channelName("Empty"), _topic(""), _mode("+nt"), _chanKey("")
{

}


Channel::Channel(std::string newChannel) : _channelName(newChannel), _topic(""), _mode("+nt"), _chanKey("")
{

}

std::ostream& operator<<(std::ostream& os, const Channel& channel){
	os << "channel name is: " << channel.getChannelName() 
		<< ", its topic is: " << channel.getTopic();
	return os;
}

std::string Channel::getChannelName() const
{
	return _channelName;
}

std::string Channel::getTopic() const
{
	return _topic;
}

Client&	Channel::getChanop() const
{
	return *_channelOperator;
}

std::vector<Client>	Channel::getUserList() const
{
	return _userList;
}

std::string	Channel::getKey() const
{
	return _chanKey;
}

void	Channel::setChanop(Client chanop)
{
	_channelOperator = &chanop;
}

void Channel::setChannelName(std::string channelName)
{
	_channelName = channelName;
}

void Channel::setTopic(std::string buffer)
{
	unsigned long topicPos = buffer.find_first_of(':');

	std::string newTopic = buffer.substr(topicPos + 1, buffer.length() - topicPos -1);
	_topic = newTopic;
}

void Channel::setKey(std::string newKey)
{
	_chanKey = newKey;
}

void Channel::addUser(Client* newClient)
{
	_userList.push_back(*newClient);
}

/**
 * @brief Check whether the client is already on the channel 
 */
bool Channel::isClientOnChannel( Client& client)
{
	for (auto it : _userList)
	{
		if (client.getNick() == it.getNick())
			return true;
	}
	return false;
}
/** @brief check if the channel key matches the key that client inputs, 
 * if channel requires a key
 */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	std::cout << "client has join " << client.getJoinedChannels().size() << " channels \n";
	if (client.getJoinedChannels().size() >= MAX_CHANNELS_PER_CLIENT)
		return TOO_MANY_CHANNELS;
	if (this->isClientOnChannel(client))
		return ALREADY_ON_CHAN;
	if (!this->getKey().empty() && this->getKey() != clientKey) // recheck with mode later
		return BAD_CHANNEL_KEY;
	return JOIN_OK;
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES). 
 */
void	Channel::sendJoinSuccessMsg( Client& client)
{

	if (!this->getTopic().empty())
	{
		std::string topicmsg 
			= this->channelMessage(CHANNEL_TOPIC_MSG, &client);
		if (send((&client)->getClientFd(), topicmsg.c_str(), topicmsg.size(), 0) < 0)
		{
			std::cout << "joinmsg: failed to send";
			close(client.getClientFd());
			return;
		}
	}
}


std::string Channel::channelMessage(channelMsg msg, Client* currentClient)
{
	std::string chanop = ":" + currentClient->getNick() + "!" 
		+ currentClient->getUserName() + "@" + currentClient->getHostName();
	std::string returnMsg;

	switch (msg)
	{
	case JOIN_OK: 	
		returnMsg = chanop + " JOIN #" + this->getChannelName() 
		+" " + RPL_TOPIC + " \r\n";
		break;

	case TOO_MANY_CHANNELS: 	
		returnMsg = ":" + currentClient->getServerName() + " " + ERR_TOOMANYCHANNELS 
		+ " " + currentClient->getNick() + " " + this->getChannelName() 
		+" :You have joined too many channels" + " \r\n";
		break;

	case BAD_CHANNEL_KEY: 	
		returnMsg = ":" + currentClient->getServerName() + " " + ERR_TOOMANYCHANNELS 
		+ " " + currentClient->getNick() + " " + this->getChannelName() 
		+" :Cannot join channel" + " \r\n";
		break;
	
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

