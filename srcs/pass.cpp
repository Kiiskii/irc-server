#include "Server.hpp"
#include "utils.hpp"

void Server::pass(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(client.getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	if (tokens.size() == 0)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "PASS");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (tokens[0].compare(_pass) == 0)
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
	else
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);					
	}	
}
