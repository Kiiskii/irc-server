#pragma once

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h> 
#include <fcntl.h>
#include <sys/epoll.h>

#include "Channel.hpp"
#include "Client.hpp"

//const char ip[]="127.0.0.1"; // for local host
#define MAX_EVENTS 200



class Client;
class Channel;

//What should be kept inside class etc...
class Server
{
	public:
	int serverfd = -1;
	std::string pass = "mouse";
	std::string name = "ft_irc";
	std::vector<Client> clientInfo;
	std::vector<Channel> channelInfo;
	const int port = 6667;
	struct sockaddr_in details;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];

	
	std::vector<Channel>::iterator isChannelExisting(std::string newChannel);
	void printChannelList() const;

};
