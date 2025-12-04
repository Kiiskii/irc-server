#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

using namespace utils;

Channel::Channel(std::string newChannel) : _channelName(newChannel), _topic("")
{
	_modeHandlers['i'] = &Channel::handleInviteOnly;
	_modeHandlers['t'] = &Channel::handleTopicRestriction; // user
	_modeHandlers['k'] = &Channel::handleChannelKey; //channel
	_modeHandlers['o'] = &Channel::handleChannelOperator; // user
	_modeHandlers['l'] = &Channel::handleChannelLimit;
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

/** @note <creationtime> is a unix timestamp */
std::string	Channel::getTopicTimestamp()
{
	return std::to_string(_topicSetTimestamp);
}

void	Channel::setTopicTimestamp()
{
	_topicSetTimestamp = time(NULL);
}

void	Channel::setChannelCreationTimestamp()
{
	_channelCreationTimestamp = time(NULL);
}

std::string	Channel::getChannelCreationTimestamp()
{
	return std::to_string(_channelCreationTimestamp);
}

Client*	Channel::getTopicSetter()
{
	return _topicSetter;
}

void	Channel::setTopicSetter(Client& setter)
{
	_topicSetter = &setter;
}

std::string	Channel::printUser() const
{
	// std::cout << "USER LIST: " << std::endl;//
	std::string returnStr = "";
	for (auto it : _ops)
	{
		if (!it){ std::cout << "no iterator exist\n"; continue;}
		returnStr += "@" + (*it).getNick() + " ";
	}
	for (auto it : _halfOps)
		returnStr += "%" + (*it).getNick() + " ";
	for (auto it : _voices)
		returnStr += "+" + (*it).getNick() + " ";
	for (auto it : _userList)
		returnStr += (*it).getNick() + " ";
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
	for (auto it  = _ops.begin(); it != _ops.end();)
	{
		if ((*it)->getNick() == opNick)
			it = _ops.erase(it);
		else
			++it;
	}
}


std::unordered_set<Client*>&	Channel::getOps()
{
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

std::vector<std::string> Channel::getMode() const
{
	// std::cout << "active mode saved size: " << _mode.size() << std::endl;
	std::string modeStr = "+";
	std::string modeArgs;
	for (auto& it : _mode)
	{
		// if (!it) { std::cout << "this mode cannot access/n"; continue; }
		// std::cout << "existing mode: key and param: [" << it.first << ", " << it.second << "]" << std::endl;
		modeStr += it.first;
		if (modeArgs.empty())
			modeArgs += it.second;
		else
			modeArgs = " " + it.second;
	}
	return {modeStr, modeArgs};
}

void Channel::addUser(Client* newClient)
{
	if (std::find(_userList.begin(), _userList.end(), newClient) != _userList.end())
		return;
	_userList.push_back(newClient);
}

void Channel::addInvitedUser(Client* newClient)
{
	_invitedUser.insert(newClient);
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
		if (utils::compareCasemappingStr(this->getChannelName(),(*chan).getChannelName()))
			return true;
	}
	return false;
}
