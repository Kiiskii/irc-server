#include "Server.hpp"

/* @def check if the channel exists
	@return ptr to channel if exist else return after the end of vector */
std::vector<Channel>::iterator Server::isChannelExisting(std::string newChannel) 
{
	for (auto it = channelInfo.begin(); it != channelInfo.end(); ++it)
	{
		if ((*it).getChannelName() == newChannel)
			return it;
	}
	return channelInfo.end();
}

void Server::printChannelList() const
{
	for (auto i : channelInfo)
	{
		std::cout << i << std::endl;
	}
}

