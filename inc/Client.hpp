#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex> //std::istringstream
#include <functional>

#include "Channel.hpp"
#include "macro.hpp"
#include "Server.hpp"

// class Channel;
// class Server;


enum ClientState
{
 NONE,
 REGISTERING,
 REGISTERED
};

class Client
{
	private:
		int						_clientfd = -1;
		std::string				_clientNick = "";
		std::string				_userName = "";
		std::string				_realName = "";
		std::string				_hostName;
		std::string				_serverName;
		std::vector<Channel*>	_joinedChannels{};
		enum ClientState		_clientState = NONE;

		std::string				_input;

	
	public:
	
//		int auth_step = 0;
		std::string recvBuffer;
		
		// Client();
		// ~Client();

		// getters
		int						getClientFd();
		std::string 			getNick();
		std::string 			getUserName();
		std::string				getRealName();
		std::string 			getHostName();
		std::string 			getServerName();
		std::vector<Channel*> 	getJoinedChannels();
		enum ClientState		getClientState();

		// setters
		void		setClientFd(int num);
		void		setNick(std::string nick);
		void		setUserName(std::string user);
		void		setRealName(std::string user);
		void		setHostName(std::string host);
		void		setServerName(std::string server);
		void		setClientState(enum ClientState state);
		void		addJoinedChannel(Channel* chan);

		// message parsing
		void recieve(Server &server, Client &c, int clientIndex);
		void parseMessage(Server &server, Client &c, const std::string &line);

		// other
		bool 		isValidJoinCmd(std::vector<std::string> tokens);

		bool		isOps(Channel& channel);

		// JOIN
		std::string makeUser();
		// void		askToJoin(std::vector<std::string> tokens, Server& server);
		
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
