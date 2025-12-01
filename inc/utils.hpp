#pragma once

#include <iostream>
#include <vector>
// #include "Channel.hpp"
// #include "Client.hpp"

class Client;
class Channel;

namespace utils {
	void		printVector(std::vector<std::string> tokens); //to remove
	void		printOps(Channel& channel); //to remove

	std::vector<std::string>	ft_splitString(std::string buffer, char delimiter);
	std::string ft_trimString(std::string msg);
	std::string ft_stringToLower(std::string str);
	std::string extractChannelName(std::string str);
	std::string setParamAndRemoveToken(std::vector<std::string>& tokens);

	bool		isValidChanName(std::string name);


	// messaging utils
	std::string makePrivMsgToChan(std::string& token, Client& client, Channel& chan);
	std::string makePrivMsgToClient(std::string& token, Client& client,Client& partner);

}

std::string	makeNumericReply(std::string prefix, int code, 
	std::string target, std::vector<std::string> params, std::string trailing);

std::string	getTarget(Client &client);
