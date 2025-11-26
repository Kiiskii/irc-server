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

static std::string makePrivMsgToChan(std::string& token, Client& client, Channel& chan)
{
	return client.makeUser() + " PRIVMSG #" +  chan.getChannelName() + " " + token + " \r\n";
}

static std::string makePrivMsgToClient(std::string& token, Client& client,Client& partner)
{
	return client.makeUser() + " PRIVMSG " + partner.getNick() + " " + token + " \r\n";;
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
		msg = makePrivMsgToChan(tokens[1], client, *chan);
		std::cout << "channel name: [" << chan->getChannelName() << "]"<< std::endl;
		std::cout << "msg: [" << msg << "]" << std::endl;
		this->broadcastChannelMsg(msg, *chan, client);

	}
	else // send privmsg to client
	{
		Client* partner = this->findClient(target);
		if (!partner)
		{
			std::cout << "client not found" << std::endl;
			return;
		}
		// msg = tokens[1];
		// if (msg[0] == ':')
		// 	msg = msg.substr(1, msg.length() - 1) + " \r\n";
		msg = makePrivMsgToClient(tokens[1], client, *partner);
		std::cout << "client name: [" << client.getNick() << "]"<< std::endl;
		std::cout << "msg: [" << msg << "]" << std::endl;
		this->sendMsg(*partner, msg);
	}
}
