#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"

void Client::kickClient(Server& server, std::vector<std::string>& params)
{
	std::string channelString = params[0];
	std::string clientString = params[1];
	std::string error;

	// check that there is correct amount of params
	if (params.size() < 3) {
		error = ERR_NEEDMOREPARAMS(server.getServerName(), getNick(), "KICK");
		server.sendMsg(*this, error);
		return ;
	}

	// check that channel exists
	Channel* chann = server.setActiveChannel(channelString);
	if (!chann) {
		server.sendClientErr(ERR_NOSUCHCHANNEL, *this, chann, {channelString});
		return ;
	}

	// check if client kicking is on the channel
	Client* cli = checkClientExistence(server.getClientInfo(), this->getNick());
	if (!cli) {
		server.sendClientErr(ERR_NOTONCHANNEL, *this, chann, {this->getNick()});
	}

	// check that client kicking is a channel operator
	if (!cli->isOps(*chann)) {
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, *this, chann, {this->getNick()});
		return ;
	}

	// check that client getting kicked is on the server
	Client* cliServer = checkClientExistence(server.getClientInfo(), clientString);
	if (!cliServer) {
		server.sendClientErr(ERR_NOSUCHNICK, *this, chann, {clientString});
		return ;
	}

	// check that client getting kicked is on the channel
	Client* cliChannel = checkClientExistence(chann->getUserList(), clientString);
	if (!cliChannel) {
		server.sendClientErr(ERR_USERNOTINCHANNEL, *this, chann, {clientString});
		return ;
	}

	server.sendKickMsg(cli->getNick(), clientString, params, *chann);

	// remove user from channelList and channel from client channelList
	chann->removeUser(clientString);
	cliChannel->removeChannel(chann);

	// if channel is empty after this, remove it from servers list of channels
	auto ite = chann->getUserList();
	if (ite.empty()) {
		server.removeChannel(chann);
	}
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
