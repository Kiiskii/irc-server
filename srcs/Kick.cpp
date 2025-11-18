#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"

// ensure that the user kicking is an operator before kicking (to be implemented)
// remove channel from clients joinedChannels list
// check that client getting kicked exists
// remove client from channels userList

bool Client::removeClient(Server& server, std::string& clientString, std::string& channelString)
{
	//auto &clients = server.getClientInfo();
	Channel* chann = setActiveChannel(channelString);
	chann.removeUser(clientString);

	/*
	decltype(clients.begin()) client = nullptr;
	for (auto it = clients.begin(); it != clients.end(); ++it) {
		if ((*it).getNick() == clientString) {
			client = it;
			break;
		}
	}
	if (client != nullptr)
		return false;
	*/
	return true;
}

void Client::kickClient(std::string &line, Server &server)
{
	std::vector<std::string> msgParts;
	msgParts = splitString(line, ' ');
	std::string currChannel = msgParts[1];
	std::string kickedUser = msgParts[2];
	std::string kickMsg;
	for (size_t i = 3; i < msgParts.size(); i++) {
		kickMsg.append(msgParts[i]);
		if (i + 1 != msgParts.size())
			kickMsg.append(" ");
	}
	std::cout << "KICK MSG: " << currChannel << ", ";
	std::cout << kickedUser << ", ";
	std::cout << kickMsg << std::endl;
}
