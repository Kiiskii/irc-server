#include "Server.hpp"
#include "utils.hpp"

/*
- User name and real name might also have some naming rules and lengths
- Need to check if either user name or real name is empty
- Check if : is requirement
- Figure out a cleaner way to extract the real name
- Do we need to show in which format this needs to be???
*/
void Server::user(Client &client, std::vector<std::string> tokens)
{
	if (tokens.size() < 4)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "USER");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	client.setUserName(tokens[0]);
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
	client.setRealName(realname);
	attemptRegister(client);
}
