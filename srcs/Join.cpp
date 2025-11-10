#include "Client.hpp"
#include "utils.hpp"


/** @brief client sends [JOIN - <channels> - <keys>] format, mapping channel-key pair to std::map and return */
static std::map<std::string, std::string> mappingChannelKey(std::string buffer)
{
	std::map<std::string, std::string>		channelKeyMap;
	std::vector<std::string>	tokens = splitString(buffer, ' ');
	bool						hasKey = false;
	
	if (tokens.size() == 3)
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

void Client::askToJoin(std::string buffer, Server& server)
{
	std::map<std::string, std::string>		channelKeyMap = mappingChannelKey(buffer);
	
	// for (auto token : channelKeyMap)
	// {
	// 	std::cout <<  "channel name - key pair: {" << token.first << ", " 
	// 		<< token.second << "}" << std::endl;
	// }

	if (validateChannelName(channelKeyMap)) // else what
	{
		for (auto chan : channelKeyMap)
		{
			std::string channelName = chan.first;
			std::string clientKey = chan.second;
			std::cout << "channel name: " << channelName << std::endl;
			std::vector<Channel*>::iterator channelNameIt 
				= server.isChannelExisting(channelName);
			Channel* channelPtr = nullptr;

			// check if the channel exists
			if (channelNameIt == server.getChannelInfo().end()) // not exist
			{
				server.getChannelInfo().push_back(new Channel(channelName));
				channelPtr = server.getChannelInfo().back();
				// this->_atChannel->setChanop(this);
			}
			else
				channelPtr = *channelNameIt;

			channelMsg result = channelPtr->canClientJoinChannel(*this, clientKey);
			if (result == JOIN_OK)
			{
				this->addChannel(channelPtr);
				channelPtr->addUser(this);
				channelPtr->sendJoinSuccessMsg(*this);
			}
			
			std::string joinMsg 
				= channelPtr->channelMessage(result, this);
			if (send(this->getClientFd(), joinMsg.c_str(), joinMsg.size(), 0) < 0)
			{
	
				std::cout << "joinmsg: failed to send";
				close(this->getClientFd());
				return; // ?? recheck this, should disconnect the client and flag to the main loop
			}
		}
	}
	// server.printChannelList(); //print all the channel on server
}