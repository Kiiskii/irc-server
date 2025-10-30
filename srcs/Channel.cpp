#include "Channel.hpp"

// mode is set to +nt for now: n - no external message, t - topic restriction
Channel::Channel() : _channelName("Empty"), _topic("Empty"), _mode("+nt")
{

}


Channel::Channel(std::string newChannel) : _channelName(newChannel), _topic("Empty"), _mode("+nt")
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
	return _key;
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
	_key = newKey;
}

void Channel::addUser(Client* newClient)
{
	_userList.push_back(*newClient);
}

std::string Channel::channelMessage(channelMsg msg, Client* currentClient)
{
	std::string chanop = ":" + currentClient->_clientNick + "!" + currentClient->_userName 
		+ "@" + currentClient->_hostName;
	std::string returnMsg;
	switch (msg)
	{
	case JOIN_MSG: 	
		returnMsg = chanop + " JOIN #" + this->getChannelName() 
		+" " + RPL_TOPIC + " \r\n";
		break;
	
	case NO_TOPIC_MSG:
		returnMsg = ":" + currentClient->_serverName + " " + RPL_NOTOPIC + " " + 
		currentClient->_clientNick + " #" + this->getChannelName() 
		+ " :No topic is set\r\n";
		break;
	
	case CHANNEL_TOPIC_MSG:
		returnMsg = ":" + currentClient->_serverName + " " + RPL_TOPIC + " " + 
		currentClient->_clientNick + " #" + this->getChannelName() 
		+ " :" + this->getTopic() +" \r\n";
		break;
	
	case WHO_CHANGE_TOPIC:
		returnMsg = ":" + currentClient->_serverName + " " + RPL_TOPICWHOTIME + " " + 
		currentClient->_clientNick + " #" + this->getChannelName() 
		+ " :" + this->getTopic() +" \r\n";
		break;
	
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

