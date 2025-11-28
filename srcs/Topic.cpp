#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/**  If <topic> is an empty string, the topic for the channel will be cleared. 
 * If the <topic> param is provided but the same as the previous topic (ie. it is unchanged), servers MAY notify the author and/or other users anyway.
 * 
*/
std::string Channel::truncateTopic(std::string tokens)
{
	std::string newTopic;
	if (tokens[0] == ':')
		newTopic = tokens.substr(1, tokens.length() - 1);
	else
		newTopic = tokens;

	int	topicLen = tokens.size();
	int maxTopic = MSG_SIZE - this->getChannelName().size() - 10;
	if (topicLen > maxTopic)
	{
		std::string truncateTopic = newTopic.substr(0, maxTopic);
		return truncateTopic;
	}
	return newTopic;
}


/** @brief if the t_mode is on, only chanop can set/remove topic 
 *	@note If <topic> is an empty string, the topic for the channel will be cleared. --> cannot test
*/
bool Channel::setTopic(std::string tokens, Client& client)
{
	Server& server = client.getServer();
	
	// std::cout << "im here setting chan name: " << tokens << std::endl;
	if (tokens.length() == 1) //there is only ":"
	{
		_topic = "";
		return true;
	}
	std::string newTopic = this->truncateTopic(tokens);

	if (this->isModeActive(T_MODE) && !client.isOps(*this))
	{
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, client, this, {});
		return false; 
	}

	_topic = newTopic;
	
	this->setTopicSetter(client);
	// time_t timestamp;
	// time(&timestamp);
	this->setTopicTimestamp();
	return true;
}

/** @brief */
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

		if (!utils::isValidChanName(channelName))
		{
			this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, {channelName});
			return;
		}
		channelPtr = this->setActiveChannel(channelName);
		// if channel not exist, send error
		if (channelPtr == nullptr)
		{
			this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, {channelName});
			return;
		}
		if (!channelPtr->isClientOnChannel(client))
		{
			this->sendClientErr(ERR_NOTONCHANNEL, client, channelPtr, {});
			return;
		}
		tokens.erase(tokens.begin(), tokens.begin() + 1);
		if (tokens.empty() && channelPtr) // ask TOPIC
			this->sendTopic(client, *channelPtr);
		else // set TOPIC
		{
			std::string topicStr;
			for (size_t i = 0; i < tokens.size(); ++i)
			{
				if (i == tokens.size() - 1)
					topicStr += tokens[i];
				else
					topicStr += tokens[i] + " ";
			}
			if (!channelPtr->setTopic(topicStr, client))
				return ;
			// this->channelMessage(CHANGE_TOPIC_MSG, &client, channelPtr);
			std::string	returnMsg = client.makeUser() + " TOPIC #" + 
			channelPtr->getChannelName() +" :" + channelPtr->getTopic() + "\r\n";
			this->broadcastChannelMsg(returnMsg, *channelPtr);
		}
	}
}
