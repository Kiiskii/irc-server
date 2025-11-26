#include "Server.hpp"
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
	/*Can we make this into a more reusable util function*/
	for (size_t i = 0; i < getClientInfo().size(); i++)
	{
		if (transformToLowercase(getClientInfo()[i]->getNick()) == transformToLowercase(tokens[0]))
		{
			std::string message = ERR_NICKNAMEINUSE(getServerName(), getTarget(client), tokens[0]);
			send(client.getClientFd(), message.c_str(), message.size(), 0);
			return ;
		}
	}
	// RFC-style implementations track “which clients share any channel” and send a single NICK to each.
	//Right now there's a bug that if you change a nick and if some channels have multiple shared channels with you, the nick change will be broadcast multiple times.
	//Also it will get broadcasted to you twice.. So need to find a way to check it only once.
	client.setNick(tokens[0]);
	if (client.getClientState() == REGISTERED)
	{
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());	
		for (Channel* channel : client.getJoinedChannels())
			this->broadcastChannelMsg(message, *channel);
	}
	if (client.getClientState() != REGISTERED)
	{
		attemptRegister(client);
	}
}
