#pragma once

#include <iostream>
#include "Channel.hpp"
#include "Client.hpp"


std::ostream&				operator<<(std::ostream& os, 
								const Channel& channel);

// std::string					ft_trimString(std::string msg);

std::vector<std::string>	splitString(std::string buffer, char delimiter);

std::string	makeNumericReply(std::string prefix, int code, 
			std::string target, std::vector<std::string> params, std::string trailing);

bool	isValidChanName(std::string name);

void	printVector(std::vector<std::string> tokens);

std::string 				getTarget(Client &client);
