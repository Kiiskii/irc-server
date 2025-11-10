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

/** @brief split string into tokens using delimiter */
static std::vector<std::string> splitString(std::string buffer, char delimiter)
{
	std::cout << "buffer :[" << buffer << "]" << std::endl;

	std::istringstream	tokenStream(buffer); //save buffer string to an istringstream obj
	std::string			aToken;
	std::vector<std::string> tokens;

	while (std::getline(tokenStream, aToken, delimiter))
	{
		tokens.push_back(aToken);
	}
	return tokens;
}

/** @brief client sends [JOIN - <channels> - <keys>] format, mapping channel-key pair to std::map and return */
static std::map<std::string, std::string> mappingChannelKey(std::string buffer)
{
	std::map<std::string, std::string>		channelKeyMap;
	std::vector<std::string>	tokens = splitString(buffer, ' ');
	bool						hasKey = false;
	
	if (tokens.size() == 3)
		hasKey = true;
	
	std::string cmd = tokens.front();
	std::vector<std::string> channelList;
	std::vector<std::string> keyList;
	if (tokens[1].find(",") != std::string::npos)
		channelList = splitString(tokens[1], ',');
	else
		channelList.push_back(tokens[1]);
	if (hasKey)
	{
		if (tokens[2].find(",") != std::string::npos)
			keyList = splitString(tokens[2], ',');
		else
			keyList.push_back(tokens[2]);
	}
	else
	{
		for (size_t i = 0; i < channelList.size(); ++i)
			keyList.push_back("");
	}
	for (size_t i = 0; i < channelList.size(); ++i)
	{
			/* The insert() operation adds a new key-value pair to the map only 
			if the key is not already present.
			If the key exists, insert() does not update the value and the map unchanged.
			IF (NUMBER OF KEYS INPUT IS SMALLER THAN NUMBER OF CHANNEL INPUT, 
			irssi sent key == "x") --> retest when finish?? */
			channelList[i].erase(0, 1);
			channelKeyMap.insert({channelList[i], keyList[i]});
		
	}
	return channelKeyMap;
}

/** @brief handle only JOIN #general, I'm not sure about JOIN '&', '+' or '!'.
	length of channel name is up to 50 chars
	shall not contain space, ascii 7 or comma 
	case insensitive*/
static bool validateChannelName(std::map<std::string, std::string> channelKeyMap)
{
	// add other check here later ??
	for (auto chan : channelKeyMap)
	{
		if (chan.first.length() > 50)
			return false;
	}
	return true;
}

void Client::askToJoin(std::string buffer, Server& server)
{
	(void)server;
	std::map<std::string, std::string>		channelKeyMap = mappingChannelKey(buffer);
	
	// for (auto token : channelKeyMap)
	// {
	// 	std::cout <<  "channel name - key pair: {" << token.first << ", " 
	// 		<< token.second << "}" << std::endl;
	// }

	if (validateChannelName(channelKeyMap)) // else what
	{
		for (auto chan : channelKeyMap)
		{
			std::string channelName = chan.first;
			std::string clientKey = chan.second;
			std::cout << "channel name: " << channelName << std::endl;
			std::vector<Channel>::iterator channelNameIt 
				= server.isChannelExisting(channelName);
			Channel* channelPtr = nullptr;

			// check if the channel exists
			if (channelNameIt == server.channelInfo.end()) // not exist
			{
				server.channelInfo.push_back(Channel(channelName));
				channelPtr = &server.channelInfo.back();
				// this->_atChannel->setChanop(this);
			}
			else
				channelPtr = &(*channelNameIt);

			channelMsg result = channelPtr->canClientJoinChannel(*this, clientKey);
			if (result == JOIN_OK)
			{
				this->addChannel(channelPtr);
				channelPtr->addUser(this);
				channelPtr->sendJoinSuccessMsg(*this);
			}
			
			std::string joinMsg 
				= channelPtr->channelMessage(result, this);
			if (send(this->getClientFd(), joinMsg.c_str(), joinMsg.size(), 0) < 0)
			{
				std::cout << "joinmsg: failed to send";
				close(this->getClientFd());
				return; // ?? recheck this, should disconnect the client and flag to the main loop
			}
		}
	}

	// server.printChannelList(); //print all the channel on server

	
}

void Client::recieve()
{
	// Recieve data from the client
	char buffer[512];
	size_t bytes = 1;
	
	// DO WE USE MSG_DONTWAIT OR 0???
	while (bytes > 0) {
		bytes = recv(getClientFd(), buffer, sizeof(buffer), MSG_DONTWAIT);

		// Errorhandling
		if (bytes < 0) {
			std::cout << "Failed to recieve from client: " << getClientFd() << std::endl;
		}
		else if (bytes == 0) {
			// disconnect client here?
			std::cout << "Client disconnect" << std::endl;
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
				parseMessage(std::string(begin, end));
				_input.erase(0, newline + 2);
			}
		}
	}
}

bool Client::parseMessage(const std::string &line)
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
		return false;

	//out.command = line.substr(cmdStart, i - cmdStart);
	msg = line.substr(cmdStart, i - cmdStart);

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
			++i;
			std::string trailing = line.substr(i);
			msg.append(trailing);
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			i = line.find(' ', i);
			msg.append(line.substr(paramStart, i - paramStart));
		}
		++j;
	}
	return true;
}

/*
#define PARTS_MAX 15

bool parseMessage(std::string &line)
{
	char* msgArray[PARTS_MAX];

	size_t i = 0;
	const size_t n = line.size();

	// skip leading spaces
	i = line.first_not_of(' ', i);

	// possibly deal with empty string?
	// if (i == n) return false;

	// command
	size_t cmdStart = i;

	i = line.find(' ', i);

	// no command
	if (cmdStart == i)
		return false;

	//out.command = line.substr(cmdStart, i - cmdStart);
	argv[0] = line.substr(cmdStart, i - cmdStart);

	// do we want to normalize to uppercase here?
	for (char &c : argv[0])
		c = std::toupper(static_cast<unsigned char>(c));

	// parameters
	int j = 1;
	while (i < n) {
		// skip spaces before next parameter
		i = line.first_not_of(' ', i);
		if (i >= n)
			break ;

		if (line[i] == ':') {
			// handle trailing after ':', trailing should always be last parameter?
			++i;
			std::string trailing = line.substr(i);
			argv[j] = trailing;
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			i = line.find(' ', i);
			agrv[j] = line.substr(paramStart, i - paramStart);
		}
		++j;
	}
	return true;
}
*/

/*
struct ParsedMessage
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

bool parseMessage(std::string &line, ParsedMessage &out)
{
	out.prefix.clear();
	out.command.clear();
	out.params.clear();

	size_t i = 0;
	const size_t n = line.size();

	// skip leading spaces
	//while (i < n && line[i] == ' ')
	//	++i;
	i = line.first_not_of(' ', i);

	// possibly deal with empty string?
	// if (i == n) return false;

	// skip optional prefix ':'
	if (line[i] == ':') {
		++i;

		size_t start = i;

		// move until space or end
		//while (i < n && line[i] != ' ')
		//	++i;
		i = line.find(' ', i);

		out.prefix = line.substr(start, i - start);

		// skip spaces after prefix
		//while (i < n && line[i] == ' ')
		//	++i;
		i = line.first_not_of(' ', i);

		if (i == n)
			return false;
	}

	// command
	size_t cmdStart = i;

	//while (i < n && line[i] != ' ')
	//	++i;
	i = line.find(' ', i);

	// no command
	if (cmdStart == i)
		return false;

	out.command = line.substr(cmdStart, i - cmdStart);

	// do we want to normalize to uppercase here?
	for (char &c : out.command)
		c = std::toupper(static_cast<unsigned char>(c));

	// parameters
	while (i < n) {
		// skip spaces before next parameter
		//while (i < n && line[i] == ' ')
		//	++i;
		i = line.first_not_of(' ', i);
		if (i >= n)
			break ;

		if (line[i] == ':') {
			// handle trailing after ':', trailing should always be last parameter?
			++i;
			std::string trailing = line.substr(i);
			out.params.push_back(trailing);
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			//while (i < n && line[i] != ' ')
			//	++i;
			i = line.find(' ', i);
			out.params.push_back(line.substr(paramStart, i - paramStart));
		}
	}
	return true;
}
*/
