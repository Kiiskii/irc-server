#include "Server.hpp"
#include "utils.hpp"

//we can adjust this into something a bit better, also do we need to fail immediately or
//fail in the attemptregister section
//giving pass again after reg just says Unknown command?
//client "keeps hanging after this", investigate
//investigate if there is a different way to handle the indexes, maybe iterator, im unsure if im using this correctly
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
	int clientIndex = 0;
	for (size_t i = 0; i < getClientInfo().size(); i++)
	{
		if (getClientInfo()[i]->getClientFd() == client.getClientFd()) // this was changed
		{
			clientIndex = i;
			break;
		}
	}
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
	}
	else if (tokens[0].compare(_pass) != 0)
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
	}
	else
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
		return ;
	}
	std::string message = ERR_GENERIC(getServerName(), getTarget(client), "Failed to match password");
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	close(client.getClientFd());
	getClientInfo().erase(iterateClients(*this, client));
}
