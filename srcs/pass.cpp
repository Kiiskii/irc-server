#include "Server.hpp"
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

void Server::disconnectClient(Client &client)
{
	auto it = iterateClients(*this, client);
	if (it == _clientInfo.end())
		return ;
	Client* ptr = *it;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, ptr->getClientFd(), NULL);
	close(ptr->getClientFd());
	getClientInfo().erase(it);
	delete ptr;
}

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
		disconnectClient(client);
	}
	else if (tokens[0].compare(_pass) != 0)
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		disconnectClient(client);
	}
	else
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
}

