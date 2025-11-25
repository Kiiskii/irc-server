#pragma once

#include <iostream>
#include "Channel.hpp"
#include "Client.hpp"


std::vector<std::string>	splitString(std::string buffer, char delimiter);

std::string	makeNumericReply(std::string prefix, int code, 
			std::string target, std::vector<std::string> params, std::string trailing);

bool		isValidChanName(std::string name);

void		printVector(std::vector<std::string> tokens);
void		printOps(Channel& channel);
std::string	getTarget(Client &client);
