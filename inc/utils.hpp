#pragma once

#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"


std::ostream&				operator<<(std::ostream& os, 
								const Channel& channel);

std::string					ft_trimString(std::string msg);

std::vector<std::string>	splitString(std::string buffer, char delimiter);

std::string					makeNumericReply(std::string prefix, int code, 
							std::string target, std::vector<std::string> params, std::string trailing);

void	printVector(std::vector<std::string> tokens);
std::string 				getTarget(Client &client);
