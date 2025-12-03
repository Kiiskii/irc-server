#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/*
- Double check the issue with possible hanging client
- Delete client..?
*/
std::vector<Client*>::iterator Server::iterateClients(Server &server, Client &client)
{
	for (auto it = _clientInfo.begin(); it != _clientInfo.end(); ++it)
	{
		if ((*it)->getClientFd() == client.getClientFd())
			return it;
	}
	return _clientInfo.end();
}

//do we disconnect???
void Server::pass(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	else if (tokens.size() == 0)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "PASS");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		client.setClientState(DISCONNECTING);
	}
	else if (tokens[0].compare(_pass) != 0)
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		client.setClientState(DISCONNECTING);
	}
	else
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
}

