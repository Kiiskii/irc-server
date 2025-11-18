#include "Server.hpp"
#include "utils.hpp"

/*Nickname rules, characters and length*/
void Server::nick(Client &client, std::vector<std::string> tokens)
{
	//so first one cannot have digits but the second one can... also added the underscore
	//this needs further investigation
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
	if (client.getClientState() == REGISTERED) // if new nick given, we need to broadcast a message
	{
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);			
	}
	attemptRegister(client);
}
