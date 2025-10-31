#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex> //std::istringstream

#include "Channel.hpp"
#include "Server.hpp"
#include "macro.hpp"

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

	// should move these to private soon
	std::string			_clientNick;
	std::string			_userName;
	std::string			_hostName;
	std::string			_serverName;
	std::vector<Channel*>	_joinedChannels; 
	
	// getters
	int			getClientFd();
	std::string getNick();
	std::string getUserName();
	std::string getHostName();
	std::string getServerName();
	std::vector<Channel*> getJoinedChannels();

	// setters
	void		setClientFd(int num);
	void		setNick(std::string nick);
	void		setUserName(std::string user);
	void		setHostName(std::string host);
	void		setServerName(std::string server);
	void		addChannel(Channel* chan);


	void 	updateClientInfo(std::string bufferStr);
	void	askToJoin(std::string buffer, Server& server);
	

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