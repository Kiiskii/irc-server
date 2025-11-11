#include "Channel.hpp"

/// mode is set to +nt for now: n - no external message, t - topic restriction
Channel::Channel() : _channelName(""), _topic("")
{
	_modeHandlers['i'] = &Channel::handleInviteOnly;
	_modeHandlers['t'] = &Channel::handleTopicRestriction; // user
	_modeHandlers['k'] = &Channel::handleChannelKey; //channel
	_modeHandlers['o'] = &Channel::handleChannelOperator; // user
	_modeHandlers['l'] = &Channel::handleChannelLimit;
}


Channel::Channel(std::string newChannel) : _channelName(newChannel), _topic("")
{
	_modeHandlers['i'] = &Channel::handleInviteOnly;
	_modeHandlers['t'] = &Channel::handleTopicRestriction; // user
	_modeHandlers['k'] = &Channel::handleChannelKey; //channel
	_modeHandlers['o'] = &Channel::handleChannelOperator; // user
	_modeHandlers['l'] = &Channel::handleChannelLimit;
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

std::string	Channel::getChanKey() const
{
	std::string chanKey = "";

	if (this->_mode.find('k') != this->_mode.end()) // k found
		chanKey = (*this->_mode.find('k')).second;
	std::cout << "channel key : [" << chanKey << "]\n"; 
	return chanKey;
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


void Channel::setChanKey(std::string newKey)
{
	this->_mode.insert({'k', newKey});
}

void Channel::addMode(char key, std::string param)
{
	_mode.insert({key, param});
}

void Channel::removeMode(char key)
{
	_mode.erase(key);
}


std::map<char, std::string> Channel::getMode() const
{
	for (auto it : _mode)
	{
		std::cout << "key and param: [" << it.first << ", " << it.second << "]" << std::endl;
	}
	return _mode;
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
	if (!this->getChanKey().empty() && this->getChanKey() != clientKey) // recheck with mode later
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
	joinInfo joinData;
	joinData.client = &client;

	if (!this->getTopic().empty())
	{
		std::string topicmsg 
			= this->channelMessage(CHANNEL_TOPIC_MSG, joinData);
		if (send((&client)->getClientFd(), topicmsg.c_str(), topicmsg.size(), 0) < 0)
		{
			std::cout << "joinmsg: failed to send";
			close(client.getClientFd());
			return;
		}
	}
}

/* @brief if this mode is set on a channel, a user must have received an INVITE for this channel before being allowed to join it. If they have not received an invite, they will receive an ERR_INVITEONLYCHAN (473) reply and the command will fail. --> when to handle client ??*/
channelMsg Channel::handleInviteOnly(bool add, std::string& args)
{
	bool active = false;
	for (auto m : _mode)
	{
		if (m.first == 'i')
		{
			active = true;
			break;
		}
	}
	if (add)
	{
		if (active)
			return NO_MSG;
		this->addMode('i', args);
		return SET_MODE_OK;
	}
	if (active)
	{
		this->removeMode('i');
		return SET_MODE_OK;
	}
	return NO_MSG;
}
channelMsg	Channel::handleTopicRestriction(bool add, std::string& args)
{
	return NO_MSG;

}
channelMsg	Channel::handleChannelKey(bool add, std::string& args)
{
	bool active = false;
	std::string key;
	for (auto m : _mode)
	{
		if (m.first == 'k')
		{
			active = true;
			key = m.second;
			break;
		}
	}
	if (add)
	{
		if (active && args == key)
			return NO_MSG;
		
		this->removeMode('k');
		this->addMode('k', args);
		return SET_MODE_OK;
		
	}
	if (active)
	{
		this->removeMode('k');
		return SET_MODE_OK; //recheck, send an empty key or nothing
	}
	return NO_MSG;
}
channelMsg	Channel::handleChannelOperator(bool add, std::string& args)
{
	return NO_MSG;
}
channelMsg	Channel::handleChannelLimit(bool add, std::string& args)
{
	return NO_MSG;

}