#include "Client.hpp"
#include "utils.hpp"

/** @note not consider the case of local channel start with '&' ?? */
static bool	isValidChanName(std::string name)
{
	std::regex chanNameRegex("^#[^ \\x07,]+$");

	if (!std::regex_match(name, chanNameRegex))
	{
		std::cout << "DOES NOT match regex pattern for channel name\n";
		return false;
	}
	return true;
}

/** @brief validate the channel name and create a map of <channel name - key> 
 *  @note The insert() operation adds a new key-value pair to the map only 
			if the key is not already present.
			If the key exists, insert() does not update the value and the map unchanged.
			IF (NUMBER OF KEYS INPUT IS SMALLER THAN NUMBER OF CHANNEL INPUT, 
			irssi sent key == "x") --> retest when finish??
			case insensitive ??
*/
static std::map<std::string, std::string> mappingChannelKey(std::vector<std::string> tokens)
{
	std::map<std::string, std::string>		channelKeyMap;
	bool						hasKey = false;

	if (tokens.size() == 2 && !tokens[1].empty())
		hasKey = true;
	
	std::vector<std::string> channelList;
	std::vector<std::string> keyList;
	if (tokens[0].find(",") != std::string::npos)
		channelList = splitString(tokens[0], ',');
	else
		channelList.push_back(tokens[0]);
	if (hasKey)
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
	for (size_t i = 0; i < channelList.size(); ++i)
	{
			if (isValidChanName(channelList[i]))
			{
				channelList[i].erase(0, 1); // remove the hash
				channelKeyMap.insert({channelList[i], keyList[i]});
			}
	}
	return channelKeyMap;
}

/** @brief check if the channel key matches the key that client inputs */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	std::cout << "client has join " << client.getJoinedChannels().size() << " channels \n";
	if (this->isClientOnChannel(client))
		return ALREADY_ON_CHAN;
	if (client.getJoinedChannels().size() >= MAX_CHANNELS_PER_CLIENT)
	{
		this->sendClientErr(ERR_TOOMANYCHANNELS, &client );
		return NO_MSG;
	}
	if (!this->getChanKey().empty() && this->getChanKey() != clientKey)
	{
		std::cout << "bad key: client key : [" << clientKey << "]\n";
		this->sendClientErr(ERR_BADCHANNELKEY, &client );
		return NO_MSG;
	}
	std::string	chanLimit;
	if (this->isModeActive(L_MODE, chanLimit))
	{
		int limit = std::stoi(chanLimit);
		std::cout << "mode L active and limit set for channel is " << limit << std::endl;
		if (this->_userList.size() >= limit)
		{
			this->sendClientErr(ERR_CHANNELISFULL, &client);
			return NO_MSG;
		}
	}
	return JOIN_OK;
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES). 
 */
void	Channel::sendJoinSuccessMsg( Client* client)
{
	std::string	user = client->makeUser();

	// send JOIN msg
	std::string joinMsg = user + " JOIN #" + this->getChannelName() 
			+ " " + std::to_string(RPL_TOPIC) + " \r\n";
	this->sendMsg(client, joinMsg);
	this->sendTopicAndNames(client);
}

bool Client::isValidJoinCmd(std::vector<std::string> tokens)
{
	if (tokens.size() < 2) //first JOIN "JOIN :", need to rethink??
		return false;
	return true;
}

void Server::handleJoin(Client* client, std::vector<std::string> tokens)
{
	// std::cout << "client has join " << this->getJoinedChannels().size() << " channels \n";
	std::cout << "client nick " << client->getNick() <<" \n";
	// if (!client.isValidJoinCmd(tokens))
	// 	return;

	std::map<std::string,std::string> channelKeyMap = mappingChannelKey(tokens);

	for (auto chan : channelKeyMap)
	{
		std::string channelName = chan.first;
		std::string clientKey = chan.second;
		std::cout << "channel name: [" << channelName << "] and key [" << clientKey << "]" << std::endl;
		std::vector<Channel*>::iterator channelNameIt 
			= this->isChannelExisting(channelName);
		Channel* channelPtr = nullptr;

		// check if the channel exists
		if (channelNameIt == this->getChannelInfo().end()) // not exist
		{
			this->getChannelInfo().push_back(new Channel(channelName));
			channelPtr = this->getChannelInfo().back();
		}
		else
			channelPtr = *channelNameIt;

		channelMsg result = channelPtr->canClientJoinChannel(*client, clientKey);
		if (result == JOIN_OK)
		{
			client->addJoinedChannel(channelPtr);
			channelPtr->addUser(client);
			if (channelPtr->getUserList().size() == 1)
				channelPtr->addChanop(client); // there is only 1 user ->ops
			channelPtr->channelMessage(result,(client));
		}
	}
	std::cout << "END JOIN \n";
	// this->printChannelList(); //print all the channel on server
}
