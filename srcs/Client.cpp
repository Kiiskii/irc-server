#include "Client.hpp"
#include <cerrno>
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(Server &server) : _myServer(server)
{

}
Client::~Client()
{

}

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

std::string	Client::getRealName()
{
	return _realName;
}
std::string Client::getHostName()
{
	return _hostName;
}

std::vector<Channel*> Client::getJoinedChannels()
{
	return _joinedChannels;
}

enum ClientState Client::getClientState()
{
	return _clientState;
}
Server& Client::getServer()
{
	return _myServer;
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

void Client::setRealName(std::string name)
{
	_realName = name;
}

void Client::setHostName(std::string host)
{
	_hostName = host;
}

void Client::setClientState(enum ClientState state)
{
	_clientState = state;
}

// this need review
void Client::addJoinedChannel(Channel* chan)
{
	if (std::find(_joinedChannels.begin(), _joinedChannels.end(), chan) != _joinedChannels.end())
		return ;
	_joinedChannels.push_back(chan);
	// std::cout << "client is added to channel\n";
}

std::string Client::makeUser()
{
	return ":" + this->getNick() + "!" 
		+ this->getUserName() + "@" + this->getHostName();
}

// this needs review
bool	Client::isOps(Channel& channel)
{
	auto it = channel.getOps().find(this);
	if (it != channel.getOps().end())
		return true;
	
	return false;
}

void Client::removeChannel(Channel* chann)
{
	for (auto it = _joinedChannels.begin(); it != _joinedChannels.end();) {
		if ((*it) == chann)
			it = _joinedChannels.erase(it);
		else
			++it;
	}
}
