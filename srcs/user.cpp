#include "Server.hpp"
#include "utils.hpp"

/*
- Max length for real name? IRC horse mentions max length for user name
*/
void Server::user(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	if (tokens.size() < 4)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "USER");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	std::string realname = "";
	if (tokens[3].find(":") != std::string::npos)
	{
		tokens[3].erase(0, tokens[3].find(":") + 1);		
	}
	for (int i = 3; i < tokens.size(); i++)
	{
		realname = realname + tokens[i];
		if (i != tokens.size() - 1)
		{
			realname = realname + " ";
		}
	}
	if (tokens[0].empty() || realname.empty())
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "USER");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	client.setUserName(tokens[0]);
	client.setRealName(realname);
	attemptRegister(client);
}
