#include "Channel.hpp"
#include "Client.hpp"

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

std::unordered_set<Client*>	Channel::getChanop() const
{
	return _ops;
}

std::vector<Client*>	Channel::getUserList() const
{
	return _userList;
}

time_t	Channel::getTopicTimestamp()
{
	return _topicSetTimestamp;
}

void	Channel::setTopicTimestamp(time_t timestamp)
{
	_topicSetTimestamp = timestamp;
}

Client*	Channel::getTopicSetter()
{
	return _topicSetter;
}

void	Channel::setTopicSetter(Client* setter)
{
	_topicSetter = setter;
}

std::string	Channel::printUser() const
{
	std::string returnStr = "";
	for (auto it : _ops)
		returnStr += "@" + (*it).getNick() + " ";
	for (auto it : _halfOps)
		returnStr += "@" + (*it).getNick() + " ";
	for (auto it : _voices)
		returnStr += "+" + (*it).getNick() + " ";
	return returnStr;
}

std::string	Channel::getChanKey() const
{
	std::string chanKey = "";

	if (this->_mode.find('k') != this->_mode.end()) // k found
		chanKey = (*this->_mode.find('k')).second;
	std::cout << "channel key : [" << chanKey << "]\n"; 
	return chanKey;
}

void Channel::addChanop(Client* chanop)
{
	_ops.insert(chanop);
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
		std::cout << "existing mode: key and param: [" << it.first << ", " << it.second << "]" << std::endl;
	}
	return _mode;
}

void Channel::addUser(Client* newClient)
{
	_userList.push_back(newClient);
}

/**
 * @brief Check whether the client is already on the channel, if already then open the window
 */
bool Channel::isClientOnChannel( Client& client)
{
	for (auto chan : client.getJoinedChannels())
	{
		if (this->getChannelName() == (*chan).getChannelName())
			return true;
	}
	
	// for (auto it : _userList)
	// {
	// 	if (client.getNick() == it->getNick())
	// 		return true;
	// }
	return false;
}

/** @brief check if the channel key matches the key that client inputs */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	std::cout << "client has join " << client.getJoinedChannels().size() << " channels \n";
	if (this->isClientOnChannel(client))
		return ALREADY_ON_CHAN;
	if (client.getJoinedChannels().size() >= MAX_CHANNELS_PER_CLIENT)
		return TOO_MANY_CHANNELS;
	if (!this->getChanKey().empty() && this->getChanKey() != clientKey)
		return BAD_CHANNEL_KEY;
	return JOIN_OK;
}

void	Channel::sendTopicAndNames(Client* client)
{
	std::string	server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName();
	// send topic / no_topic
	if (this->getTopic().empty())
	{
		std::string topicMsg = makeNumericReply(server, RPL_NOTOPIC, nick, 
			{"#" + chanName}, "No topic is set");
		this->sendMsg(client, topicMsg);
	}
	else
	{
		std::string topicMsg = makeNumericReply(server, RPL_TOPIC, nick, 
			{"#" + chanName}, this->getTopic());
		this->sendMsg(client, topicMsg);
		
		// below not test yet
		std::time_t timestamp = this->getTopicTimestamp();
		std::string topicWhoMsg = makeNumericReply(server, RPL_TOPICWHOTIME,
			nick, {"#" + chanName, getTopicSetter()->getNick(), std::to_string(timestamp)}, "");
		this->sendMsg(client, topicWhoMsg);
	}

	// send name list and end of list
	std::string nameReplyMsg = makeNumericReply(server, RPL_NAMREPLY, nick,  {"=", "#"+ chanName}, this->printUser() );
	this->sendMsg(client, nameReplyMsg);

	std::string endOfNamesMsg = makeNumericReply(server,
	RPL_ENDOFNAMES,	nick, {"#" + chanName},
	"End of /NAMES list.");
	this->sendMsg(client, endOfNamesMsg);
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES). 
 */
void	Channel::sendJoinSuccessMsg( Client* client)
{
	std::string	user = client->makeUser();

	// send JOIN msg
	std::string joinMsg = user + " JOIN #" + this->getChannelName() 
			+ " " + std::to_string(RPL_TOPIC) + " \r\n";
	this->sendMsg(client, joinMsg);
	this->sendTopicAndNames(client);
}

/**
 * @brief send message to the joining member, does it go to all members??
 */
void	Channel::sendMsg(Client* client, std::string& msg)
{
	if (send(client->getClientFd(), msg.c_str(), msg.size(), 0) < 0)
	{
		std::cout << "joinmsg: failed to send";
		close(client->getClientFd()); //do i need to close, cause then other functions after this will continue on closed client
		return;
	}
}
// /**
//  * @brief send message to all member on channels and the joining member itself
//  */
// void	Channel::broadcastMsg(Client& client, std::string& msg)
// {
// 	for (auto it : client.getServerName())
// 	if (send((&client)->getClientFd(), msg.c_str(), msg.size(), 0) < 0)
// 	{
// 		std::cout << "joinmsg: failed to send";
// 		close(client.getClientFd()); //do i need to close, cause then other functions after this will continue on closed client
// 		return;
// 	}
// }


/**	@brief if this mode is set on a channel, a user must have received an INVITE for this channel before being allowed to join it. If they have not received an invite, they will receive an ERR_INVITEONLYCHAN (473) reply and the command will fail. --> when to handle client ?? */
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