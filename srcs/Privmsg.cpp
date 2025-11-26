#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

using namespace utils;

static bool isValidPrivmsg(Client& client, std::vector<std::string>& tokens) 
{
	if (tokens.size() < 2)
	{
		std::string msg = ERR_NEEDMOREPARAMS(client.getServer().getServerName(), client.getNick(), "PRIVMSG");
		client.getServer().sendMsg(client, msg);
		return false;
	}


	return true;
}

void Server::handlePrivmsg(Client& client, std::vector<std::string> tokens)
{
	if (!isValidPrivmsg(client, tokens))
		return;
	std::string target = tokens[0];
	std::string	msg;

	if (target.find("#") != std::string::npos) // send privmsg to channel
	{
		Channel* chan = this->setActiveChannel(target);
		if (!chan)
		{
			std::cout << "channel not found" << std::endl;
			return;
		}
		msg = tokens[1];
		if (msg[0] == ':')
			msg = msg.substr(1, msg.length() - 1) + " \r\n";
		std::cout << "channel name: [" << chan->getChannelName() << "]"<< std::endl;
		std::cout << "msg: [" << msg << "]" << std::endl;
		this->broadcastChannelMsg(tokens[1], *chan);

	}
	else // send privmsg to client
	{
		Client* partner = this->findClient(target);
		if (!partner)
		{
			std::cout << "channel not found" << std::endl;
			return;
		}
		msg = tokens[1];
		if (msg[0] == ':')
			msg = msg.substr(1, msg.length() - 1) + " \r\n";
		std::cout << "client name: [" << client.getNick() << "]"<< std::endl;
		std::cout << "msg: [" << msg << "]" << std::endl;
		this->sendMsg(*partner, tokens[1]);
	}
}
