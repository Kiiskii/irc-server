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

// Channel::~Channel()
// {
// 	for (auto user :_ops)
// 		delete user;
// 	for (auto user :_voices)
// 		delete user;
// 	for (auto user :_halfOps)
// 		delete user;
// 	for (auto user :_invitedUser)
// 		delete user;
// 	for (auto user :_normals)
// 		delete user;
// 	for (auto user :_userList)
// 		delete user;
// }

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
	return {modeStr, modeArgs};
}

void Channel::addUser(Client* newClient)
{
	// for (auto user : _userList)
	// {
	// 	if (utils::compareCasemappingStr(user->getNick(), newClient->getNick()))
	// 		return;
	// }
	if (std::find(_userList.begin(), _userList.end(), newClient) != _userList.end())
		return ;
	_userList.push_back(newClient);
	if (_userList.size() == 1)
		this->addChanop(newClient);
	else
		this->addNormal(newClient);
}

void Channel::addChanop(Client* chanop)
{
	if (std::find(_ops.begin(), _ops.end(), chanop) != _ops.end())
		return ;
	// for (auto op : _ops)
	// {
	// 	if (utils::compareCasemappingStr(op->getNick(), chanop->getNick()))
	// 		return;
	// }
	_ops.insert(chanop);
}

void Channel::addNormal(Client* client)
{
	if (std::find(_normals.begin(), _normals.end(), client) != _normals.end())
		return ;
	// for (auto normal : _normals)
	// {
	// 	if (utils::compareCasemappingStr(normal->getNick(), client->getNick()))
	// 		return;
	// }
	_normals.insert(client);
}

void Channel::addInvitedUser(Client* newClient)
{
	// for (auto user : _invitedUser)
	// {
	// 	if (utils::compareCasemappingStr(user->getNick(), newClient->getNick()))
	// 		return;
	// }
	if (std::find(_invitedUser.begin(), _invitedUser.end(), newClient) != _invitedUser.end())
		return ;
	_invitedUser.insert(newClient);
}

void	Channel::removeUser(std::string userNick)
{
	// for (auto it  = _userList.begin(); it != _userList.end();)
	// {
	// 	if ((*it)->getNick() == userNick)
	// 		it = _userList.erase(it);
	// 	else
	// 		++it;
	// }
	Client* client = this->findClient(userNick);
	if (client)
	{
		_userList.erase(std::remove(_userList.begin(), _userList.end(), client), _userList.end());
	}
	_ops.erase(client);
	_normals.erase(client);
	_invitedUser.erase(client);
}

/** @brief remove the client pointer from chanops list using NICK */
void	Channel::removeChanop(std::string opNick)
{
	// for (auto it  = _ops.begin(); it != _ops.end();)
	// {
	// 	if (utils::compareCasemappingStr((*it)->getNick(), opNick))
	// 		it = _ops.erase(it);
	// 	else
	// 		++it;
	// }
	Client* client = this->findClient(opNick);
	if (client)
		_ops.erase(client);
}

void	Channel::removeNormal(std::string userNick)
{
	// for (auto it  = _normals.begin(); it != _normals.end();)
	// {
	// 	if (utils::compareCasemappingStr((*it)->getNick(), userNick))
	// 		it = _normals.erase(it);
	// 	else
	// 		++it;
	// }
	Client* client = this->findClient(userNick);
	if (client)
		_normals.erase(client);
}

/** @brief Check whether the client is already on the channel, 
 * if already on the channel then open the window */
bool Channel::isClientOnChannel( Client& client)
{
	for (auto chan : client.getJoinedChannels())
	{
		// if (utils::compareCasemappingStr(this->getChannelName(),(*chan).getChannelName()))
		if (this == chan)
			return true;
	}
	return false;
}
// correct
bool	Channel::isChanop(std::string nick)
{
	for (auto op : _ops)
	{
		if (utils::compareCasemappingStr(op->getNick(), nick))
			return true;
	}
	return false;
}
// correct
Client*	Channel::findClient(std::string nickName)
{
	for (auto it = _userList.begin(); it != _userList.end(); ++it)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), nickName))
			return *it;
	}
	return nullptr;
}