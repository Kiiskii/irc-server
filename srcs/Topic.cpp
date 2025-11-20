#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"



/** @brief if the t_mode is on, only chanop can set/remove topic */
void Channel::setTopic(std::string buffer, Client& client)
{
	Server& server = client._myServer;
	// not test this yet
	if (this->isModeActive(T_MODE) && !client.isOps(*this))
	{
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, client, *this, {});
		return; 
	}
	unsigned long topicPos = buffer.find_first_of(':');
	std::string newTopic = buffer.substr(topicPos + 1, 
							buffer.length() - topicPos -1);
	_topic = newTopic;
	
	this->setTopicSetter(client);
	time_t timestamp;
	time(&timestamp);
	this->setTopicTimestamp(timestamp);
}

/** @note what to do when having too many params for topic ?? */
void Server::handleTopic(Client& client, std::vector<std::string> tokens)
{
	Channel* channelPtr;
	std::string	channelName;

	if (tokens.empty())
	{
		std::string msg = ERR_NEEDMOREPARAMS(this->getServerName(), client.getNick(), "TOPIC");
		this->sendMsg(client , msg);
		return;
	}

	if (tokens.size() > 0)
	{
		channelName = tokens[0];
		channelPtr = this->setActiveChannel(channelName);
		// if not on any channel, return do nothing
		if (channelPtr == nullptr || !channelPtr->isClientOnChannel(client))
		{
			this->sendClientErr(ERR_NOTONCHANNEL, client, *channelPtr, {});
			return;
		}
		tokens.erase(tokens.begin(), tokens.begin() + 1);
		if (tokens.empty()) // ask TOPIC
		{
			if (channelPtr && channelPtr->getTopic().empty())
				this->sendNoTopic(client, *channelPtr);
			else if (channelPtr && !channelPtr->getTopic().empty())
				this->sendTopic(client, *channelPtr);
		}
		else // set TOPIC
		{
			std::string topicStr;
			for (size_t i = 0; i < tokens.size(); ++i)
			{
				if (i == tokens.size() - 1)
					topicStr += tokens[i];
				topicStr += tokens[i] + " ";
			}
			std::cout << "im here setting chan name: " << topicStr << std::endl;
			channelPtr->setTopic(topicStr, client);
			// std::cout << "im here sending chan name: " << tokens[1] << std::endl;
			this->channelMessage(CHANGE_TOPIC_MSG, &client, channelPtr);
		}
	}
}
