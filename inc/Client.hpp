#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex> //std::istringstream
#include <functional>

#include "utils.hpp"
#include "macro.hpp"

class Channel;
class Server;

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

		std::vector<Channel*>	_joinedChannels{};
		enum ClientState		_clientState = NONE;
		class Server			&_myServer;	
//		std::string				_input;

	
	public:
		std::string				_input; // this needs to be private
//		int auth_step = 0;
		std::string recvBuffer;
		
		Client(Server &server);
		~Client();

		// getters
		int						getClientFd();
		std::string 			getNick();
		std::string 			getUserName();
		std::string				getRealName();
		std::string 			getHostName();
		// std::string 			getServerName();
		std::vector<Channel*> 	getJoinedChannels();
		enum ClientState		getClientState();
		Server&					getServer();

		// setters
		void		setClientFd(int num);
		void		setNick(std::string nick);
		void		setUserName(std::string user);
		void		setRealName(std::string user);
		void		setHostName(std::string host);
		void		setClientState(enum ClientState state);
		void		addJoinedChannel(Channel* chan);

		// message parsing


		// other
		bool		isOps(Channel& channel);
		std::string makeUser();
		
		// TOPIC
		Channel*	setActiveChannel(std::string buffer);
		void		askTopic(std::string buffer);

		// MODE
		void		changeMode(std::string buffer, Server& server);	
		void		kickClient(std::string &line, Server &server);
		bool		removeClient(Server& server, std::string& clientString, std::string& channelString);
	

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
