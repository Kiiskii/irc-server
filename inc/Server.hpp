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

#include "Enum.hpp"
#include "utils.hpp"
#include "macro.hpp"

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
	std::string _name = "";
	std::vector<Client*> _clientInfo;
	std::vector<Channel*> _channelInfo;
	int _port = -1;
	struct sockaddr_in _details;
	struct epoll_event _event;
	struct epoll_event _events[MAX_EVENTS];

public:
	~Server();
//getters
	int getServerfd() const;
	int getEpollfd() const;
	std::string& getServerName();
	struct epoll_event* getEpollEvents();
	std::vector<Client*>& getClientInfo();
	std::vector<Channel*>& getChannelInfo();

	void setupServerDetails(Server &server, int argc, char *argv[]);
	void setupSocket();
	void setupEpoll();
	void handleNewClient();
	void handleClient();
	//void handleCommand(Server &server, Client &client, std::string &line);
	void handleCommand(Server &server, Client &client, std::string command, std::vector<std::string> &tokens);
	void attemptRegister(Client &client);
	void disconnectClient(Client &client);

/*Commands such as user, pass nick, might be best to create a separate place for commands*/
	void pass(Client &client, std::vector<std::string> tokens);
	void nick(Client &client, std::vector<std::string> tokens);
	void user(Client &client, std::vector<std::string> tokens);
	void ping(Client &client, std::vector<std::string> tokens);
	std::vector<Client*>::iterator 	iterateClients(Server &server, Client &client);
	std::vector<Channel*>::iterator isChannelExisting(std::string newChannel);
	void handleJoin(Client& client, std::vector<std::string> tokens);
	void handleTopic(Client& client, std::vector<std::string> tokens);
	void handleMode(Client& client, std::vector<std::string> tokens);

	Channel* findChannel(std::string newChannel);
	void printChannelList() const;

// Server message to client
	bool 		mappingChannelKey(std::vector<std::string> tokens, Client& client, 
					std::map<std::string, std::string>& channelKeyMap);
	Channel*	setActiveChannel(std::string buffer);
	void		sendMsg(Client& client, std::string& msg);
	void		sendTopic(Client& client, Channel& channel);
	void		sendJoinSuccessMsg( Client& client, Channel& channel);
	void		sendNameReply(Client& client, Channel& channel);
	void		broadcastChannelMsg(std::string& msg, Channel& channel);
	void		sendClientErr(int num, Client& client, Channel* channel, 
					std::vector<std::string> otherArgs);

	
	template <typename ...args>
	void		channelMessage(channelMsg msg, args ...moreArgs);

};

#include "Server.tpp"
