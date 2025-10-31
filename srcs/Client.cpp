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
void Client::updateClientInfo(std::string bufferStr)
{
	(void) bufferStr;
	_clientNick = "trpham";
	_userName ="trpham";
	_hostName = "localhost";
	_serverName = "localhost";
}

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