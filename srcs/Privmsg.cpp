#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

using namespace utils;

static bool isValidPrivmsg(Client& client, std::vector<std::string>& tokens) 
{
	if (tokens.size() < 3)
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
	
}
