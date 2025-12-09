#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

using namespace utils;

Channel::Channel(std::string newChannel) : _channelName(newChannel), _topic("")
{
	_modeHandlers['i'] = &Channel::handleInviteOnly;
	_modeHandlers['t'] = &Channel::handleTopicRestriction;
	_modeHandlers['k'] = &Channel::handleChannelKey;
	_modeHandlers['o'] = &Channel::handleChannelOperator;
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
	std::string returnStr = "";
	for (auto it : _ops)
		returnStr += "@" + (*it).getNick() + " ";
	for (auto it : _halfOps)
		returnStr += "%" + (*it).getNick() + " ";
	for (auto it : _voices)
		returnStr += "+" + (*it).getNick() + " ";
	for (auto it : _normals)
		returnStr += (*it).getNick() + " ";
	return returnStr;
}

std::string	Channel::getChanKey() const
{
	std::string chanKey = "";

	if (this->_mode.find(K_MODE) != this->_mode.end()) // k found
		chanKey = (*this->_mode.find('k')).second;
	return chanKey;
}

void Channel::addChanop(Client* chanop)
{
	for (auto op : _ops)
	{
		if (utils::compareCasemappingStr(op->getNick(), chanop->getNick()))
			return;
	}
	// std::cout << "add op with nick: " << chanop->getNick() << std::endl;
	_ops.insert(chanop);
}

void Channel::addNormal(Client* client)
{
	for (auto normal : _normals)
	{
		if (utils::compareCasemappingStr(normal->getNick(), client->getNick()))
			return;
	}
	_normals.insert(client);
}

/** @brief remove the client pointer from chanops list using NICK */
void	Channel::removeChanop(std::string opNick)
{
	for (auto it  = _ops.begin(); it != _ops.end();)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), opNick))
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
	this->_mode.insert({K_MODE, newKey});
}

void Channel::addMode(char key, std::string param)
{
	_mode.insert({key, param});
}

/** @note map remove the key and its value if key exists, else does nothing */
void Channel::removeMode(char key)
{
	_mode.erase(key);
}

std::vector<std::string> Channel::getMode() const
{
	std::string modeStr = "+";
	std::string modeArgs;
	for (auto& it : _mode)
	{
		modeStr += it.first;
		if (modeArgs.empty())
			modeArgs += it.second;
		else
			modeArgs += " " + it.second;
	}
	std::cout << "mode str: " << modeStr << " mode arg: " << modeArgs << std::endl;
	return {modeStr, modeArgs};
}

void Channel::addUser(Client* newClient)
{
	for (auto user : _userList)
	{
		if (utils::compareCasemappingStr(user->getNick(), newClient->getNick()))
			return;
	}
	_userList.push_back(newClient);
}

void Channel::addInvitedUser(Client* newClient)
{
	for (auto user : _invitedUser)
	{
		if (utils::compareCasemappingStr(user->getNick(), newClient->getNick()))
			return;
	}
	_invitedUser.insert(newClient);
}

void	Channel::removeUser(std::string userNick)
{
	for (auto it  = _userList.begin(); it != _userList.end();)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), userNick))
			it = _userList.erase(it);
		else
			++it;
	}
}

void	Channel::removeNormal(std::string userNick)
{
	for (auto it  = _normals.begin(); it != _normals.end();)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), userNick))
			it = _normals.erase(it);
		else
			++it;
	}
}

/** @brief Check whether the client is already on the channel, 
 * if already on the channel then open the window */
bool Channel::isClientOnChannel( Client& client)
{
	for (auto chan : client.getJoinedChannels())
	{
		if (utils::compareCasemappingStr(this->getChannelName(),(*chan).getChannelName()))
			return true;
	}
	return false;
}

bool	Channel::isChanop(std::string nick)
{
	for (auto op : _ops)
	{
		if (utils::compareCasemappingStr(op->getNick(), nick))
			return true;
	}
	return false;
}

Client*	Channel::findClient(std::string nickName)
{
	for (auto it = _userList.begin(); it != _userList.end(); ++it)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), nickName))
			return *it;
	}
	return nullptr;
}