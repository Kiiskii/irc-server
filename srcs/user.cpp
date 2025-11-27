#include "Server.hpp"
#include "Client.hpp"
#include "utils.hpp"

/*
- Max length for user name? But where do we get user info
- Right now we are removing the :, does it need to be so?
*/
void Server::user(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	if (tokens.size() == 4 && tokens[3].size() != 0 && tokens[3].find(":") != std::string::npos)
	{
		tokens[3].erase(0, tokens[3].find(":") + 1);		
	}
	if (tokens.size() < 4 || tokens[0].empty() || tokens[3].empty())
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "USER");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (tokens[0].size() > USERLEN)
		client.setUserName(tokens[0].substr(0, USERLEN));
	else
		client.setUserName(tokens[0]);
	client.setRealName(tokens[3]);
	attemptRegister(client);
}
