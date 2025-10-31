#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex> //std::istringstream

#include "Channel.hpp"
#include "Server.hpp"

class Channel;
class Server;

class Client
{
	private:
	
	public:
	int auth_step = 0;
	int clientfd = -1;
	std::string user = "";
	std::string nick = "";
	// channelInfo *channel = nullptr;
	
	void 	updateClientInfo(std::string bufferStr);
	void	askToJoin(std::string buffer, Server& server);
	
	std::string			_clientNick;
	std::string			_userName;
	std::string			_hostName;
	std::string			_serverName;
	// Channel*			_atChannel;
	std::vector<Channel*>	_joinedChannels; 

};

/* 

The server does the following:

Check if the channel exists

If it doesn’t exist, usually the server creates the channel with default modes (and no key unless specified).

Check channel modes

If the channel has mode +k (keyed), the server compares the key provided by the client with the channel’s key.

Decision:

If no key is required (+k not set) → client can join immediately.

If key is required (+k) → client can only join if the key matches exactly.

If the key does not match → the server responds with:
*/