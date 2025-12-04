#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"
#include "macro.hpp"

void Client::kickClient(Server& server, std::vector<std::string>& params)
{
	std::string error;

	// check that there is correct amount of params
	if (params.size() < 2) {
		error = ERR_NEEDMOREPARAMS(server.getServerName(), getNick(), "KICK");
		server.sendMsg(*this, error);
		return ;
	}

	std::string channelString = params[0];
	std::string clientString = params[1];

	// check that channel exists
	Channel* chann = server.setActiveChannel(channelString);
	if (!chann) {
		server.sendClientErr(ERR_NOSUCHCHANNEL, *this, chann, {channelString});
		return ;
	}

	// check if client kicking is on channel
	if (!chann->isClientOnChannel(*this)) {
		server.sendClientErr(ERR_NOTONCHANNEL, *this, chann, {this->getNick()});
		return ;
	}

	// check that client kicking is a channel operator
	if (!this->isOps(*chann)) {
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, *this, chann, {this->getNick()});
		return ;
	}

	// check for multiple clients and put them into a vector
	std::vector<std::string> clientList;
	if (clientString.find(",") != std::string::npos)
		clientList = utils::ft_splitString(clientString, ',');
	else
		clientList.push_back(clientString);

	for (size_t i = 0; i < clientList.size(); ++i) {

		// check that client getting kicked is on the server
		Client* cliServer = checkClientExistence(server.getClientInfo(), clientList[i]);
		if (!cliServer) {
			server.sendClientErr(ERR_NOSUCHNICK, *this, chann, {clientList[i]});
			return ;
		}

		// check that client getting kicked is on the channel
		Client* cliChannel = checkClientExistence(chann->getUserList(), clientList[i]);
		if (!cliChannel) {
			server.sendClientErr(ERR_USERNOTINCHANNEL, *this, chann, {clientList[i]});
			return ;
		}

		server.sendKickMsg(this->getNick(), clientList[i], params, *chann);

		// remove user from channelList and channel from client channelList
		chann->removeUser(clientList[i]);
		cliChannel->removeChannel(chann);
	}

	// if channel is empty after this, remove it from servers list of channels
	auto ite = chann->getUserList();
	if (ite.empty()) {
		server.removeChannel(chann);
	}
}

void Client::partChannel(Server& server, std::vector<std::string>& params)
{
	std::string error;

	// check that there is correct amount of params
	if (params.size() == 0) {
		error = ERR_NEEDMOREPARAMS(server.getServerName(), getNick(), "KICK");
		server.sendMsg(*this, error);
		return ;
	}

	std::string channelString = params[0];

	// check that channel exists
	Channel* chann = server.setActiveChannel(channelString);
	if (!chann) {
		server.sendClientErr(ERR_NOSUCHCHANNEL, *this, chann, {channelString});
		return ;
	}

	// check if client kicking is on channel
	if (!chann->isClientOnChannel(*this)) {
		server.sendClientErr(ERR_NOTONCHANNEL, *this, chann, {this->getNick()});
		return ;
	}

	server.sendPartMsg(*this, params, *chann);

	// remove user from channelList and channel from client channelList
	chann->removeUser(this->getNick());
	this->removeChannel(chann);

	// if channel is empty after this, remove it from servers list of channels
	auto ite = chann->getUserList();
	if (ite.empty()) {
		server.removeChannel(chann);
	}

}
