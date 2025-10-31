#pragma once

#include <iostream>
#include <vector>
#include "Client.hpp"
#include "macro.hpp"


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
	ALREADY_ON_CHAN

};

class Client;

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
		std::string			_channelName;
		std::string			_topic;
		Client*				_channelOperator; //previledge (chanop, voiced user)
		std::vector<Client> _userList; //who in channel
		std::string			_mode; //what mode
		std::string			_chanKey;
		
	public:

	
		Channel();
		~Channel() = default;
		Channel(std::string newChannel);

		// getters
		std::string 		getChannelName() const;
		std::string 		getTopic() const;
		Client&				getChanop() const;
		std::vector<Client>	getUserList() const;
		std::string			getKey() const;

		// setters
		void		setChannelName(std::string channelName);
		void		setChanop(Client chanop);
		void		setTopic(std::string newTopic);
		void		addUser(Client* newClient);
		void		setKey(std::string newKey);

		// channel public method
		bool		isClientOnChannel( Client& client);
		channelMsg	canClientJoinChannel( Client& client, std::string clientKey);
		void		sendJoinSuccessMsg( Client& client);
		
		std::string channelMessage(channelMsg msg,  Client* currentClient);
		// void		handleJoinCmd(std::string buffer, Client& currentClient);
		// unsigned int	checkTopicCmd(std::string buffer);

		

	
};

std::ostream& operator<<(std::ostream& os, const Channel& channel);
