#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

std::string transformToLowercase(std::string string)
{
	transform(string.begin(), string.end(), string.begin(), [](char c)
	{
		return tolower(c);
	});
	return string;
}

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
	if (std::regex_match(tokens[0], pattern) == false || tokens[0].length() > NICKLEN)
	{
		std::string message = ERR_ERRONEUSNICKNAME(getServerName(), getTarget(client), tokens[0]);
		send(client.getClientFd(), message.c_str(), message.size(), 0);	
		return ;
	}
	if (tokens[0] == client.getNick()) //we should not inform that nickname is alrdy in use if you are trying to change your nick to nick you currently have
		return ;
	for (size_t i = 0; i < getClientInfo().size(); i++) //if my nickname is already X and I try to change my nick to X again, I should not get a warning in that case
	{
		if (transformToLowercase(getClientInfo()[i]->getNick()) == transformToLowercase(tokens[0]))
		{
			std::string message = ERR_NICKNAMEINUSE(getServerName(), getTarget(client), tokens[0]);
			send(client.getClientFd(), message.c_str(), message.size(), 0);
			return ;
		}
	}
	client.setNick(tokens[0]);
	if (client.getClientState() == REGISTERED)
	{
		std::vector<int> uniqueClients;
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());	
		for (Channel* channel : client.getJoinedChannels())
		{
			for (Client* user : channel->getUserList())
			{
				if (auto it = find(uniqueClients.begin(), uniqueClients.end(), user->getClientFd()) == uniqueClients.end())
				{
					uniqueClients.push_back(user->getClientFd());
					send(user->getClientFd(), message.c_str(), message.size(), 0);
				}
			}
		}
		if (client.getJoinedChannels().size() == 0)
		{
			send(client.getClientFd(), message.c_str(), message.size(), 0);
		}
	}
	if (client.getClientState() != REGISTERED)
	{
		attemptRegister(client);
	}
}

//do we want to use unordered map...
	// 	if (client.getClientState() == REGISTERED)
	// {
	// 	std::unordered_map<int, Client*> uniqueClients;
	// 	std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());	
	// 	for (Channel* channel : client.getJoinedChannels())
	// 	{
	// 		for (Client* user : channel->getUserList())
	// 		{
	// 			if (auto it = find(uniqueClients.begin(), uniqueClients.end(), user->getClientFd()) == uniqueClients.end());
	// 				uniqueClients.insert({user->getClientFd(), user});
	// 		}
	// 	}
	// }
