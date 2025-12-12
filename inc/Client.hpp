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
 REGISTERED,
 DISCONNECTING
};

class Client
{
	private:
		int						_clientfd = -1;
		std::string				_clientNick = "";
		std::string				_userName = "";
		std::string				_realName = "";
		std::string				_hostName;

		std::vector<Channel*>	_joinedChannels;
		enum ClientState		_clientState = NONE;
		class Server			&_myServer;	
//		std::string				_input;

	public:
		std::string				_input; // this needs to be private
//		int auth_step = 0;
		std::string 			recvBuffer;
		Client() = delete;
		Client(Server &server);
		~Client() = default;

		// getters
		int						getClientFd();
		std::string 			getNick();
		std::string 			getUserName();
		std::string				getRealName();
		std::string 			getHostName();
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

		// other methods
		void		addJoinedChannel(Channel* chan);
		bool		isOps(Channel& channel);
		std::string makeUser();
		bool		isValidChanName(std::string name);
		void		removeChannel(Channel* chann);
	
};
