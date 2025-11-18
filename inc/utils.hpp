#pragma once

#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

enum	channelMsg
{
	NO_MSG,
	JOIN_OK,
	ALREADY_ON_CHAN,
	CHANGE_TOPIC_MSG,
	//below not use
	INVITE_ONLY_CHAN,

	// mode response, should move out??
	SET_MODE_OK,
	NO_ACTION,
	UNKNOWN_MODE,
	

};


std::ostream&				operator<<(std::ostream& os, 
								const Channel& channel);

std::string					ft_trimString(std::string msg);

std::vector<std::string>	splitString(std::string buffer, char delimiter);

std::string					makeNumericReply(std::string prefix, int code, 
							std::string target, std::vector<std::string> params, std::string trailing);

void	printVector(std::vector<std::string> tokens);
std::string 				getTarget(Client &client);
