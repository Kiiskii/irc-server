#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <utility>
#include <unistd.h>
#include <sys/socket.h> 
// #include "Client.hpp"
#include "macro.hpp"

class Client;

enum	channelMsg
{
	NO_MSG,
	NO_TOPIC_MSG,
	CHANNEL_TOPIC_MSG,
	WHO_CHANGE_TOPIC,
	NAME_LIST_MSG,
	CHANGE_TOPIC_MSG,
	// JOIN MSG
	JOIN_OK,
	// NO_SUCH_CHANNEL, // may not need cause creating new channel anyway
	TOO_MANY_CHANNELS,
	BAD_CHANNEL_KEY,
	INVITE_ONLY_CHAN,
	ALREADY_ON_CHAN,
	SET_MODE_OK

};

// struct joinInfo { 
// 	Client* client;
	
// };

// struct topicInfo { 
// 	Client* client;
// 	std::string topic;
// };

// struct modeInfo { 
// 	Client*		client;
// 	char		addMode;
// 	char		mode;
// 	std::string	param;
// 	channelMsg	msgEnum;
// };




/*
	@brief The channel is created implicitly when the first client joins it, 
	and the channel ceases to  exist when the last client leaves it
	Channels names are strings (beginning with a '&' or '#' character) of
   length up to 200 characters.
    Apart from the the requirement that the
   first character being either '&' or '#'; the only restriction on a
   channel name is that it may not contain any spaces (' '), a control G
   (^G or ASCII 7), or a comma (',' which is used as a list item
   separator by the protocol).
   A channel operator is identified by the '@' symbol next to their
   nickname whenever it is associated with a channel
   Because of IRC's scandanavian origin, the characters {}| are
   considered to be the lower case equivalents of the characters []\,
   respectively. This is a critical issue when determining the
   equivalence of two nicknames.
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
		std::unordered_set<Client*>	_ops; //previledge(chanop,halfop,voiced??)
		std::unordered_set<Client*>	_halfOps; // set allows uniqueness
		std::unordered_set<Client*>	_voices;
		std::vector<Client*> 		_userList; //who in channel
		std::map<char, std::string>	_mode; //mode: itkol
		std::map<char, channelMsg (Channel::*)(bool, std::string&)> _modeHandlers;
		// std::string			_chanKey;
		
	public:

	
		Channel();
		~Channel() = default;
		Channel(std::string newChannel);

		// getters
		std::string 		getChannelName() const;
		std::string 		getTopic() const;
		std::unordered_set<Client*>	getChanop() const;
		std::vector<Client*>		getUserList() const;
		std::string			getChanKey() const;
		// std::map<char,std::string> getMode() const;
		std::map<char,std::string> getMode() const;

		// setters
		void		setChannelName(std::string channelName);
		void		addChanop(Client* chanop);
		void		setTopic(std::string newTopic);
		void		addUser(Client* newClient);
		void		removeUser(Client* user);
		void		setChanKey(std::string newKey);
		void 		addMode(char key, std::string param);
		void		removeMode(char key);

		// channel public method
		bool		isClientOnChannel( Client& client);
		channelMsg	canClientJoinChannel( Client& client, std::string clientKey);
		void		sendJoinSuccessMsg( Client& client);
		
		// template
		template <typename ...args>
		std::string channelMessage(channelMsg msg, args ...moreArgs);

		// mode
		void			setMode(std::string buffer, Client* client);
		void			setMode(std::string buffer, channelMsg& msgEnum, std::string& modeStatus, std::string& params);
		channelMsg		handleInviteOnly(bool add, std::string& args);
		channelMsg		handleTopicRestriction(bool add, std::string& args);
		channelMsg		handleChannelKey(bool add, std::string& args);
		channelMsg		handleChannelOperator(bool add, std::string& args);
		channelMsg		handleChannelLimit(bool add, std::string& args);
	
};

std::ostream& operator<<(std::ostream& os, const Channel& channel);

#include "Channel.tpp"
