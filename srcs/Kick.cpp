#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"

// ensure that the user kicking is an operator before kicking (to be implemented)
// remove channel from clients joinedChannels list
// check that client getting kicked exists
// remove client from channels userList

void Client::kickClient(Server& server, std::vector<std::string>& params)
{
	std::string channelString = params[0];
	std::string clientString = params[1];
	Client* c = nullptr;
	Channel* chann = server.setActiveChannel(channelString);
	if (!chann) {
		std::cout << "Channel does not exist." << std::endl;
		return ;
	}
	for (auto it:chann->getUserList()) {
		if ((*it).getNick() == clientString) {
			c = it;
			break ;
		}
	}
	if (!c) {
		std::cout << "User not found in channel." << std::endl;
		return ;
	}
	chann->removeUser(clientString);
	c->removeChannel(chann);
	auto ite = chann->getUserList();
	std::cout << server.getChannelInfo().size() << std::endl;
	if (ite.empty()) {
		server.removeChannel(chann);
	}
	std::cout << server.getChannelInfo().size() << std::endl;
	return ;
}

/*
bool Client::removeClient(Server& server, std::string& clientString, std::string& channelString)
{
	Client* c = nullptr;
	Channel* chann = server.setActiveChannel(channelString);
	if (!chann) {
		std::cout << "Channel does not exist." << std::endl;
		return false;
	}
	for (auto it:chann->getUserList()) {
		if ((*it).getNick() == clientString) {
			c = it;
			break ;
		}
	}
	if (!c) {
		std::cout << "User not found in channel." << std::endl;
		return false;
	}
	chann->removeUser(clientString);
	c->removeChannel(chann);
	server.printChannelList();
	if (chann.empty())
		server.removeChannel(chann);
	server.printChannelList();
	std::cout << "JOINED CHANNELS AFTER: ";
	for (auto ite:c->getJoinedChannels()) {
		std::cout << (*ite).getChannelName() << " / ";
	}
	std::cout << std::endl;
	return true;
}
*/

/*
void Client::kickClient(std::vector<std::string> &msg, Server &server)
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
	removeClient(server, kickedUser, currChannel);
	std::cout << "KICK MSG: " << currChannel << ", ";
	std::cout << kickedUser << ", ";
	std::cout << kickMsg << std::endl;
}
*/
