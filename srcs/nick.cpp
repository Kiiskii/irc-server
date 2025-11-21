#include "Server.hpp"
#include "utils.hpp"

/*
- Case insensitivity? Is Karoliina considered the same as KAROLIINA?
- Do we need to have max length?
- What about control characters?
- Also we may want to add if size is bigger than 1 check to the regex match if statement, in case person gives a nick name that is divided by space (separate tokens)
*/
void Server::nick(Client &client, std::vector<std::string> tokens)
{
	std::regex pattern(R"(^[A-Za-z\[\]{}\\|_][A-Za-z0-9\[\]{}\\|_]*$)");
	std::string oldnick = client.getNick();
	if (tokens.size() == 0)
	{
		std::string message = ERR_NONICKNAMEGIVEN(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (std::regex_match(tokens[0], pattern) == false)
	{
		std::string message = ERR_ERRONEUSNICKNAME(getServerName(), getTarget(client), tokens[0]);
		send(client.getClientFd(), message.c_str(), message.size(), 0);	
		return ;
	}
	/*Can we make this into a more reusable util function*/
	for (size_t i = 0; i < getClientInfo().size(); i++)
	{
		if (getClientInfo()[i]->getNick() == tokens[0])
		{
			std::string message = ERR_NICKNAMEINUSE(getServerName(), getTarget(client), tokens[0]);
			send(client.getClientFd(), message.c_str(), message.size(), 0);
			return ;
		}
	}
	client.setNick(tokens[0]);
	if (client.getClientState() == REGISTERED)
	{
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);	
	}
	if (client.getClientState() != REGISTERED)
	{
		attemptRegister(client);
	}
}
