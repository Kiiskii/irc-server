#pragma once

#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

// class 

std::ostream& operator<<(std::ostream& os, const Channel& channel);


// channelMsg checkTopicComd(std::string bufferStr, Client* currentClient, Channel* currentChan);
std::string ft_trimString(std::string msg);
std::vector<std::string> splitString(std::string buffer, char delimiter);