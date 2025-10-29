#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "Channel.hpp"

class Channel;

class Client
{
	public:
	int auth_step = 0;
	int clientfd = -1;
	std::string user = "";
	std::string nick = "";
	// channelInfo *channel = nullptr;

	std::string	_clientNick;
	std::string _userName;
	std::string _hostName;
	std::string _serverName;
	Channel*	_atChannel = {0};

	void updateClientInfo(std::string bufferStr);
};