#pragma once

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <sys/types.h> 
#include <fcntl.h>
#include <sys/epoll.h>
#include "macro.hpp"

#include "Channel.hpp"
#include "Client.hpp"

//const char ip[]="127.0.0.1"; // for local host
#define MAX_EVENTS 200



class Client;
class Channel;

class Server
{
	private:
	int _epollFd = -1;
	int _serverFd = -1;
	std::string _pass = "";
	std::string _name = "ft_irc";
	std::vector<Client> _clientInfo;
	std::vector<Channel> _channelInfo;
	int _port = -1;
	struct sockaddr_in _details;
	struct epoll_event _event;
	struct epoll_event _events[MAX_EVENTS];

	public:
//getters
	int getServerfd() const;
	int getEpollfd() const;
	struct epoll_event* getEpollEvents();
	std::vector<Client>& getClientInfo();
	std::vector<Channel>& getChannelInfo();

	void setupServerDetails(Server &server, int argc, char *argv[]);
	void setupSocket();
	void setupEpoll();
	void handleNewClient();
	void handleClient();
	void handleCommand(Server &server, Client &client, std::string &line);
	std::vector<Channel>::iterator isChannelExisting(std::string newChannel);
	void printChannelList() const;

};
