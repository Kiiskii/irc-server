#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"

/** @brief validate the channel name and create a map of <channel name - key> 
 *  @note The insert() operation adds a new key-value pair to the map only 
			if the key is not already present.
			If the key exists, insert() does not update the value and the map unchanged.
			IF (NUMBER OF KEYS INPUT IS SMALLER THAN NUMBER OF CHANNEL INPUT, 
			irssi sent key == "x") --> retest when finish??
*/
bool Server::mappingChannelKey(std::vector<std::string> tokens, Client& client, std::map<std::string, std::string>& channelKeyMap)
{
	// std::string msg;

	if (tokens.empty())
	{
		std::string msg = ERR_NEEDMOREPARAMS(client.getServer().getServerName(), client.getNick(), "JOIN");
		client.getServer().sendMsg(client, msg);
		return false;
	}
	
	std::vector<std::string> channelList;
	std::vector<std::string> keyList;
	if (tokens[0].find(",") != std::string::npos)
		channelList = splitString(tokens[0], ',');
	else
		channelList.push_back(tokens[0]);
	if (tokens.size() > 1) //has key
	{
		if (tokens[1].find(",") != std::string::npos)
			keyList = splitString(tokens[1], ',');
		else
			keyList.push_back(tokens[1]);
	}
	else
	{
		for (size_t i = 0; i < channelList.size(); ++i)
			keyList.push_back("");
	}
	// add to map
	for (size_t i = 0; i < channelList.size(); ++i)
	{
		if (isValidChanName(channelList[i]))
		{
			channelList[i].erase(0, 1); // remove the hash
			channelKeyMap.insert({channelList[i], keyList[i]});
		}
		else
		{
			this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, 
				{"#" + channelList[i]});
			continue;
		}
	}
	return true;
}

/** @brief check if the channel key matches the key that client inputs */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	Server& server = client.getServer();
	std::cout << "client has join " << client.getJoinedChannels().size() << " channels \n";
	if (this->isClientOnChannel(client))
	{
		std::cout << "client is already on channel\n";
		return ALREADY_ON_CHAN;
	}

	if (client.getJoinedChannels().size() >= MAX_CHANNELS_PER_CLIENT)
	{
		server.sendClientErr(ERR_TOOMANYCHANNELS, client, this, {} );
		return NO_MSG;
	}

	if (!this->getChanKey().empty() && this->getChanKey() != clientKey)
	{
		std::cout << "bad key: client key : [" << clientKey << "]\n";
		server.sendClientErr(ERR_BADCHANNELKEY,client, this, {} );
		return NO_MSG;
	}

	std::string	chanLimit;
	if (this->isModeActive(L_MODE, chanLimit))
	{
		int limit = std::stoi(chanLimit);
		std::cout << "mode L active and limit set for channel is " << limit << std::endl;
		if (this->_userList.size() >= limit && !this->hasInvitedClient(&client)) //invitation will bypass the limit of channel
		{
			server.sendClientErr(ERR_CHANNELISFULL, client, this, {});
			return NO_MSG;
		}
	}

	if (this->isModeActive(I_MODE))
	{
		std::cout << "Invite-only Mode active " << std::endl;
		if (!this->hasInvitedClient(&client))
		{
			server.sendClientErr(ERR_INVITEONLYCHAN, client, this, {});
			return NO_MSG;
		}
	}
	
	return JOIN_OK;
}

/** @note JOIN 0 will leave all the channels -> how?? 
 * regular channel: This channel is what’s referred to as a normal channel. Clients can join this channel, and the first client who joins a normal channel is made a channel operator, along with the appropriate channel membership prefix. On most servers, newly-created channels have then protected topic "+t" and no external messages "+n" modes enabled, but exactly what modes new channels are given is up to the server. 
*/
void Server::handleJoin(Client& client, std::vector<std::string> tokens)
{
	// std::cout << "client has join " << this->getJoinedChannels().size() << " channels \n";
	std::map<std::string, std::string>		channelKeyMap;

	if (!mappingChannelKey(tokens, client, channelKeyMap))
		return;

	for (auto chan : channelKeyMap)
	{
		std::string channelName = chan.first;
		std::string clientKey = chan.second;
		// std::cout << "channel name: [" << channelName << "] and key [" << clientKey << "]" << std::endl;

		// check if the channel existschannel.
		Channel* channelPtr = this->findChannel(channelName);
		if (!channelPtr)
		{
			this->getChannelInfo().push_back(new Channel(channelName));
			channelPtr = this->getChannelInfo().back();
		}

		channelMsg result = channelPtr->canClientJoinChannel(client, clientKey);
		if (result == JOIN_OK)
		{
			client.addJoinedChannel(channelPtr);
			channelPtr->addUser(&client);
			if (channelPtr->getUserList().size() == 1)
				channelPtr->addChanop(&client); // there is only 1 user ->ops
			this->channelMessage(result, &client, channelPtr);
		}
		
		std::cout << "[" << channelPtr->printUser() << "]" << std::endl; //remove
		channelPtr->getOps(); //remove
	}

}
