#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include <utility>
#include <unistd.h>
#include <sys/socket.h> 
#include <ctime>

#include "Enum.hpp"

class Client;
class Server;

/*
	@brief The channel is created implicitly when the first client joins it, 
	and the channel ceases to  exist when the last client leaves it
	Channels names are strings (beginning with a '&' or '#' character)
	CHANLIMIT=50  CHANNELLEN=50 TOPICLEN=390 CASEMAPPING=ascii??

	IRC message has 3 parts: 512 characters including /r/n
	- prefix(optional): :<prefix> <message>
	- command
	- command parameters(upt to 15)
*/
class Channel
{
	private:
		std::string					_channelName;
		std::string					_topic;
		std::unordered_set<Client*>	_ops; //set allows uniqueness, but is it necessar?
		std::unordered_set<Client*>	_halfOps; 
		std::unordered_set<Client*>	_voices;
		std::unordered_set<Client*>	_invitedUser;
		std::vector<Client*> 		_userList;
		std::map<char, std::string>	_mode;
		std::map<char, channelMsg (Channel::*)(bool, std::string&)> _modeHandlers;
		time_t						_topicSetTimestamp;
		Client*						_topicSetter;
		time_t						_channelCreationTimestamp;
		
	public:

		Channel() = delete;
		~Channel() = default;
		Channel(std::string newChannel);

		// getters
		std::string 				getChannelName() const;
		std::string 				getTopic() const;
		std::vector<Client*>&		getUserList() ;
		std::string					getChanKey() const;
		std::vector<std::string> 	getMode() const;
		std::string					printUser() const;
		std::string					getTopicTimestamp();
		std::string					getChannelCreationTimestamp();
		Client*						getTopicSetter();
		std::unordered_set<Client*>&	getOps();


		// setters
		void			setChannelName(std::string channelName);
		bool			setTopic(std::string newTopic, Client& clientset);
		void			addUser(Client* newClient);
		void			addInvitedUser(Client* newClient);
		void			removeUser(std::string userNick);
		void			setChanKey(std::string newKey);
		void 			addMode(char key, std::string param);
		void			removeMode(char key);
		void			setTopicTimestamp();
		void			setTopicSetter(Client& setter);
		void			setChannelCreationTimestamp();

		// channel public method
		bool			isClientOnChannel( Client& client);
		channelMsg		canClientJoinChannel( Client& client, std::string clientKey);

		// mode
		bool			setMode(std::vector<std::string>& tokens, Client& client);
		bool			isModeActive(char mode);
		bool			isModeActive(char mode, std::string& key);
		channelMsg		handleInviteOnly(bool add, std::string& args);
		channelMsg		handleTopicRestriction(bool add, std::string& args);
		channelMsg		handleChannelKey(bool add, std::string& args);
		channelMsg		handleChannelOperator(bool add, std::string& args);
		channelMsg		handleChannelLimit(bool add, std::string& args);
		void			addChanop(Client* chanop);
		void			removeChanop(std::string opNick);
		bool			hasInvitedClient(Client* client);
		bool			isValidModeCmd(std::string modeStr, Client& client);
		std::string		truncateTopic(std::string name);

};


