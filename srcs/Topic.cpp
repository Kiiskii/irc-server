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

	channelPtr = client.setActiveChannel(channelName);
	// if not on any channel, return do nothing
	if (channelPtr == nullptr)
	{
		return;
	}

	if (tokens.size() > 0)
	{
		channelName = tokens[0];
	}

	if (tokens.size() == 1)
	{
		
		if (channelPtr && channelPtr->getTopic().empty())
			this->sendNoTopic(client, *channelPtr);
		else if (channelPtr && !channelPtr->getTopic().empty())
			this->sendTopic(client, *channelPtr);
	}
    else if (tokens.size() == 2)
    {
        // std::cout << "im here setting chan name: " << tokens[1] << std::endl;
        channelPtr->setTopic(tokens[1], client);
        // std::cout << "im here sending chan name: " << tokens[1] << std::endl;
		this->channelMessage(CHANGE_TOPIC_MSG, &client, channelPtr);
    }
	else
	{
		std::cout << "too many params\n";
		return;
	}
}
