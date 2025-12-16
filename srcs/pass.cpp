#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/*
- Double check the issue with possible hanging client
- Check if we already have iterate clients function
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

void Server::pass(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), getTarget(client));
		sendMsg(client, message);
		return ;	
	}
	else if (tokens.size() == 0)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), getTarget(client), "PASS");
		sendMsg(client, message);
		client.setClientState(DISCONNECTING);
	}
	else if (tokens[0].compare(_pass) != 0)
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		sendMsg(client, message);
		client.setClientState(DISCONNECTING);
	}
	else
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
}

