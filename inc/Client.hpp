#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "Channel.hpp"

class Channel;

class Client
{
private:

public:
	Client() = default;
	~Client() = default;
	Client(int fd) {
		clientfd = fd;
	}

	int32_t		clientfd = -1;
	std::string	_clientNick;
	std::string _userName;
	std::string _hostName;
	std::string _serverName;
	Channel*	_atChannel = {0};

	void updateClientInfo(std::string bufferStr);
	
};
