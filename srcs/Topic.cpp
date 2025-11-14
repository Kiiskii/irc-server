#include "Client.hpp"
#include "utils.hpp"


Channel* Client::setActiveChannel(std::string buffer)
{
	// Channel* channelPtr = nullptr;
	std::string	channelName;

	size_t hashPos = buffer.find("#");
	if (hashPos == std::string::npos)
		return nullptr;
	
	size_t chanEndPos = buffer.find(' ', hashPos);
	if (chanEndPos == std::string::npos)
		chanEndPos = buffer.length();

	channelName = buffer.substr(hashPos + 1, chanEndPos - hashPos -1);
	std::cout << "channelName: [" << channelName << "]" << std::endl;
	for (auto chan : this->_joinedChannels)
	{
		if (chan && chan->getChannelName() == channelName)
			return chan;
		else
		{
			std::cout << "there is no channel saved in _joinedChannel" << std::endl;
		}
	}

	return nullptr;
}

void Channel::setTopic(std::string buffer, Client* client)
{
	// not test this yet
	if (!this->canNonOpsSetTopic() && !client->isOps(this))
	{
		this->sendOpPrivsNeeded(client); 
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

void Client::askTopic(std::string buffer)
{
	Channel* channelPtr;

	std::cout << "channel size: " << this->_joinedChannels.size() << std::endl;
	channelPtr = setActiveChannel(buffer);
	// if not on any channel, return do nothing
	if (channelPtr == nullptr)
		return;
	// client hasn't joined channel
	if (!channelPtr->isClientOnChannel(*this))
	{
		std::string notOnChanMsg = makeNumericReply(this->getServerName(), 
			ERR_NOTONCHANNEL, this->getNick(), {"#" + channelPtr->getChannelName()}, "You're not on that channel");
		channelPtr->sendMsg(this, notOnChanMsg);
		return;
	}

	if (buffer == "TOPIC")
	{
		if (channelPtr && channelPtr->getTopic().empty())
			channelPtr->sendNoTopic(this);
		else if (channelPtr && !channelPtr->getTopic().empty())
			channelPtr->sendTopic(this);
	}
    else if (buffer.find(":") != std::string::npos)
    {
        // std::cout << "im here setting chan name: " << std::endl;
        channelPtr->setTopic(buffer, this);
		channelPtr->channelMessage(CHANGE_TOPIC_MSG, this);
    }
}