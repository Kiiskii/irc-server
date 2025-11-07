#include "Client.hpp"

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

void Client::addChannel(Channel* chan)
{
	_joinedChannels.push_back(chan);
}

/**
 * @brief need to fix this one, currently fix value for channel testing */
//Do we need this?
// void Client::updateClientInfo(std::string bufferStr)
// {
// 	(void) bufferStr;
// 	// _clientNick = "trpham";
// 	// _userName ="trpham";
// 	_hostName = "localhost";
// 	_serverName = "localhost";
// }

