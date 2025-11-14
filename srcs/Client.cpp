#include "Client.hpp"
#include "Channel.hpp"

int	Client::getClientFd()
{
	return _clientfd;
}

std::string Client::getNick()
{
	return _clientNick;
}

std::string Client::getUserName()
{
	return _userName;
}
std::string Client::getHostName()
{
	return _hostName;
}

std::string Client::getServerName()
{
	return _serverName;
}

std::vector<Channel*> Client::getJoinedChannels()
{
	return _joinedChannels;
}

enum ClientState Client::getClientState()
{
	return _clientState;
}

void Client::setClientFd(int num)
{
	_clientfd = num;
}

void Client::setNick(std::string nick)
{
	_clientNick = nick;
}

void Client::setUserName(std::string user)
{
	_userName = user;
}

void Client::setHostName(std::string host)
{
	_hostName = host;
}

void Client::setServerName(std::string server)
{
	_serverName = server;
}

void Client::setClientState(enum ClientState state)
{
	_clientState = state;
}

void Client::addChannel(Channel* chan)
{
	_joinedChannels.push_back(chan);
}

std::string Client::makeUser()
{
	return ":" + this->getNick() + "!" 
		+ this->getUserName() + "@" + this->getHostName();
}

bool	Client::isOps(Channel* channel)
{
	auto it = channel->getOps().find(this);
	if (it != channel->getOps().end())
		return true;
	return false;
}
