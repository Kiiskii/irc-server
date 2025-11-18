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


std::vector<Client*>&	Channel::getUserList() 
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
	std::cout << "ENTER PRINT USER: " << std::endl;
	std::string returnStr = "";
	for (auto it : _ops)
	{
		if (!it){ std::cout << "no iterator exist\n"; continue;}
		std::cout << "OP PTR=" << it << std::endl;
   		std::cout << "NICK=" << it->getNick() << std::endl;
		returnStr += "@" + (*it).getNick() + " ";
	}
	for (auto it : _halfOps)
		returnStr += "%" + (*it).getNick() + " ";
	for (auto it : _voices)
		returnStr += "+" + (*it).getNick() + " ";
	std::cout << "print user not break: " << returnStr << std::endl;
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

void	Channel::removeChanop(std::string opNick)
{
	for (auto op : _ops)
	{
		if (op->getNick() == opNick)
			_ops.erase(op);
	}
}


std::unordered_set<Client*>&	Channel::getOps()
{
	std::cout << "operators list: \n";
	if (!_ops.empty())
	{
		for (auto op : _ops)
		{
			std::cout << op->getNick() << ", ";
		}
		std::cout << "\n";
	}
	return _ops;
}

void Channel::setChannelName(std::string channelName)
{
	_channelName = channelName;
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

void	Channel::removeUser(std::string userNick)
{
	for (auto it  = _userList.begin(); it != _userList.end();)
	{
		if ((*it)->getNick() == userNick)
			it = _userList.erase(it);
		else
			++it;
	}
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
	return false;
}

/**
 * @brief send message to the joining member, does it go to all members??
 */
void	Channel::sendMsg(Client* client, std::string& msg)
{
	std::cout << "we here: " << std::endl;
	if (send(client->getClientFd(), msg.c_str(), msg.size(), 0) < 0)
	{
		std::cout << "joinmsg: failed to send\n";
		return;
	}
	std::cout << "msg sent: " << msg << std::endl;
}
/**
 * @brief send message to all member on channels and the joining member itself
 */
void Channel::broadcastChannelMsg(std::string& msg)
{
	for (Client* user : this->_userList)
		this->sendMsg(user, msg);
	//recheck does this send to the joining memeber itself
}

// template <typename ...args>
void Channel::sendClientErr(int num, Client* client)
{
	std::string server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName(),
				msg, extraArg;
	
	// auto				tupleArgs = std::make_tuple(moreArgs);
	// constexpr size_t	nArgs = sizeof...(moreArgs);

	// if constexpr (nArgs > 0)
	// 	extraArg = std::get<0>tupleArgs;

	switch (num)
	{
	case ERR_BADCHANNELKEY:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+k)");
		break;

	case ERR_TOOMANYCHANNELS:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "You have joined too many channels");
		break;

	case ERR_UNKNOWNMODE:
		msg = makeNumericReply(server, num, nick, {}, "is unknown mode char to me");
		break;

	case ERR_CHANNELISFULL:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+l)");
		break;

	case ERR_INVITEONLYCHAN:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "Cannot join channel (+i)");
		break;
	
	case ERR_NOTONCHANNEL:
		msg = makeNumericReply(server, num, nick, {"#" + chanName}, "You're not on that channel");
		break;

	case 461:
		msg = ERR_NEEDMOREPARAMS(server, client->getNick(),"MODE"); // need fix
		// msg = makeNumericReply(server, num, nick, {"MODE"}, "Not enough parameters");
		break;	
	
	default:
		break;
	}
	this->sendMsg(client, msg);
}

