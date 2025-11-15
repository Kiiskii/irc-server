#include "Client.hpp"
#include "utils.hpp"


/** @brief client sends [JOIN - <channels> - <keys>] format, mapping channel-key pair to std::map and return */
static std::map<std::string, std::string> mappingChannelKey(std::string buffer)
{
	std::map<std::string, std::string>		channelKeyMap;
	std::vector<std::string>	tokens = splitString(buffer, ' ');
	bool						hasKey = false;

	if (tokens.size() == 3 && !!tokens[2].empty())
		hasKey = true;
	
	std::string cmd = tokens.front();
	std::vector<std::string> channelList;
	std::vector<std::string> keyList;
	if (tokens[1].find(",") != std::string::npos)
		channelList = splitString(tokens[1], ',');
	else
		channelList.push_back(tokens[1]);
	if (hasKey)
	{
		if (tokens[2].find(",") != std::string::npos)
			keyList = splitString(tokens[2], ',');
		else
			keyList.push_back(tokens[2]);
	}
	else
	{
		for (size_t i = 0; i < channelList.size(); ++i)
			keyList.push_back("");
	}
	for (size_t i = 0; i < channelList.size(); ++i)
	{
			/* The insert() operation adds a new key-value pair to the map only 
			if the key is not already present.
			If the key exists, insert() does not update the value and the map unchanged.
			IF (NUMBER OF KEYS INPUT IS SMALLER THAN NUMBER OF CHANNEL INPUT, 
			irssi sent key == "x") --> retest when finish?? */
			channelList[i].erase(0, 1);
			channelKeyMap.insert({channelList[i], keyList[i]});
		
	}
	return channelKeyMap;
}

/** @brief handle only JOIN #general, I'm not sure about JOIN '&', '+' or '!'.
	length of channel name is up to 50 chars
	shall not contain space, ascii 7 or comma 
	case insensitive*/
static bool validateChannelName(std::map<std::string, std::string> channelKeyMap)
{
	// add other check here later ??
	for (auto chan : channelKeyMap)
	{
		if (chan.first.length() > 50)
			return false;
	}
	return true;
}

/** @brief check if the channel key matches the key that client inputs */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	std::cout << "client has join " << client.getJoinedChannels().size() << " channels \n";
	if (this->isClientOnChannel(client))
		return ALREADY_ON_CHAN;
	if (client.getJoinedChannels().size() >= MAX_CHANNELS_PER_CLIENT)
	{
		// return TOO_MANY_CHANNELS;
		this->sendClientErr(ERR_TOOMANYCHANNELS, &client );
		return NO_MSG;
	}
	if (!this->getChanKey().empty() && this->getChanKey() != clientKey)
	{
		// return BAD_CHANNEL_KEY;
		this->sendClientErr(ERR_BADCHANNELKEY, &client );
		return NO_MSG;
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

bool Client::isValidJoinCmd(std::string buffer)
{
	if (buffer.find(":") != std::string::npos) //first JOIN "JOIN :"
		return false;
	return true;
}

void Client::askToJoin(std::string buffer, Server& server)
{
	// std::cout << "client has join " << this->getJoinedChannels().size() << " channels \n";
	if (!isValidJoinCmd(buffer))
		return;

	std::map<std::string,std::string> channelKeyMap = mappingChannelKey(buffer);

	if (validateChannelName(channelKeyMap)) // else what
	{
		for (auto chan : channelKeyMap)
		{
			std::string channelName = chan.first;
			std::string clientKey = chan.second;
			// std::cout << "channel name: " << channelName << std::endl;
			std::vector<Channel*>::iterator channelNameIt 
				= server.isChannelExisting(channelName);
			Channel* channelPtr = nullptr;

			// check if the channel exists
			if (channelNameIt == server.getChannelInfo().end()) // not exist
			{
				server.getChannelInfo().push_back(new Channel(channelName));
				channelPtr = server.getChannelInfo().back();
			}
			else
				channelPtr = *channelNameIt;

			channelMsg result = channelPtr->canClientJoinChannel(*this, clientKey);
			if (result == JOIN_OK)
			{
				this->addJoinedChannel(channelPtr);
				channelPtr->addUser(this);
				if (channelPtr->getUserList().size() == 1)
					channelPtr->addChanop(this); // there is only 1 user ->ops
			}
			channelPtr->channelMessage(result, this);
		}
	}
	// server.printChannelList(); //print all the channel on server
}