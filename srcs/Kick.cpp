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

	// do we need to check if client is on the server?
	//Client* cli = checkClientExistence(server.getClientInfo(), this->getNick());
	//if (!cli) {
	//	server.sendClientErr(ERR_NOTONCHANNEL, *this, chann, {this->getNick()});
	//	return ;
	//}

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

	server.sendKickMsg(this->getNick(), clientString, params, *chann);

	// remove user from channelList and channel from client channelList
	chann->removeUser(clientString);
	cliChannel->removeChannel(chann);

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
