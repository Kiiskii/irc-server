#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex> //std::istringstream
#include <functional>

#include "Channel.hpp"
#include "Server.hpp"
#include "macro.hpp"

class Channel;
class Server;


enum ClientState
{
 NONE,
 GOT_PASS,
 GOT_NICK,
 GOT_USER,
 REGISTERED
};

class Client
{
	private:
		int						_clientfd = -1;
		std::string				_clientNick;
		std::string				_userName;
		std::string				_hostName;
		std::string				_serverName;
		std::vector<Channel*>	_joinedChannels;
		enum ClientState		_clientState = NONE;

	
	public:
	
//		int auth_step = 0;
		std::string recvBuffer;
		
		// getters
		int						getClientFd();
		std::string 			getNick();
		std::string 			getUserName();
		std::string 			getHostName();
		std::string 			getServerName();
		std::vector<Channel*> 	getJoinedChannels();
		enum ClientState		getClientState();

		// setters
		void		setClientFd(int num);
		void		setNick(std::string nick);
		void		setUserName(std::string user);
		void		setHostName(std::string host);
		void		setServerName(std::string server);
		void		setClientState(enum ClientState state);
		void		addChannel(Channel* chan);

		// message parsing
		//bool parseMessage(std::string &line, ParsedMessage &out);

		// other
		// void		updateClientInfo(std::string bufferStr);
		// JOIN
		void		askToJoin(std::string buffer, Server& server);
		
		// TOPIC
		Channel*	setActiveChannel(std::string buffer);
		void		askTopic(std::string buffer);
		// MODE
		void		changeMode(std::string buffer);	
	

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
