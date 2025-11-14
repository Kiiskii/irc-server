#include "Client.hpp"
#include <cerrno>
#include "Channel.hpp"
#include "Server.hpp"

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

void Client::setRealName(std::string name)
{
	_realName = name;
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

void Client::recieve(Server &server, Client &c, int clientIndex)
{
	// Recieve data from the client
	char buffer[512];
	ssize_t bytes = 1;

	//outMsg.clear();
	
	// DO WE USE MSG_DONTWAIT OR 0???
	while (bytes > 0) {
		bytes = recv(getClientFd(), buffer, sizeof(buffer), MSG_DONTWAIT);
		// Errorhandling
		if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
			std::cout << "Failed to recieve from client: " << getClientFd() << std::endl;
			break ;
		}
		else if (bytes == 0) {
/*Cleaner way to handle this rather than sending in index*/
			std::cout << "Client fd " << getClientFd() << " disconnected" << std::endl;
			close(getClientFd());
			server.getClientInfo().erase(server.getClientInfo().begin() + clientIndex);
			break ;
		}
		// Buffer recieved data
		else {
			_input.append(buffer, bytes);

			// Check if message if complete
			while (true) {
				size_t newline = _input.find("\r\n");
				if (newline == _input.npos)
					break ;
				auto begin = _input.begin();
				auto end = _input.begin() + newline;
				parseMessage(server, c, std::string(begin, end));
				_input.erase(0, newline + 2);
			}
		}
	}
}

void Client::parseMessage(Server &server, Client &c, const std::string &line)
{
	std::string msg;

	size_t i = 0;
	const size_t n = line.size();

	// skip leading spaces
	i = line.find_first_not_of(' ', i);

	// possibly deal with empty string?
	// if (i == n) return false;

	// command
	size_t cmdStart = i;

	i = line.find(' ', i);

	// no command
	if (cmdStart == i)
		return ;

	//out.command = line.substr(cmdStart, i - cmdStart);
	msg = line.substr(cmdStart, i - cmdStart) + ' ';

	// do we want to normalize to uppercase here?
	for (char &c : msg)
		c = std::toupper(static_cast<unsigned char>(c));

	// parameters
	int j = 1;
	while (i < n) {
		// skip spaces before next parameter
		i = line.find_first_not_of(' ', i);
		if (i >= n)
			break ;

		if (line[i] == ':') {
			// handle trailing after ':', trailing should always be last parameter?
			//++i;
			std::string trailing = line.substr(i);
			msg.append(trailing);
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			i = line.find(' ', i);
			msg.append(line.substr(paramStart, i - paramStart) + ' ');
		}
		++j;
	}
	if (!msg.empty() && msg.back() == ' ')
		msg.pop_back();
	server.handleCommand(server, c, msg);
}
bool	Client::isOps(Channel* channel)
{
	auto it = channel->getOps().find(this);
	if (it != channel->getOps().end())
		return true;
	return false;
}
