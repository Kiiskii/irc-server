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
		{
			// channelPtr = chan;
			// std::cout << "current channel name: " << chan->getChannelName() << std::endl;
			return chan;
		}
		else
		{
			std::cout << "there is no channel saved in _joinedChannel" << std::endl;
		}
	}

	return nullptr;
}

void Client::askTopic(std::string buffer)
{
	Channel* channelPtr;
	channelMsg result;

	std::cout << "channel size: " << this->_joinedChannels.size() << std::endl;
	channelPtr = setActiveChannel(buffer);
	// if not on any channel, return do nothing
	if (channelPtr == nullptr)
		return;

	if (buffer == "TOPIC")
	{
		if (channelPtr && channelPtr->getTopic().empty())
			result = NO_TOPIC_MSG;
		else if (channelPtr && !channelPtr->getTopic().empty())
			result = CHANNEL_TOPIC_MSG;
	}
    else if (buffer.find(":") != std::string::npos)
    {
        // std::cout << "im here setting chan name: " << std::endl;
        channelPtr->setTopic(buffer);
        std::cout << "topic after set: " << channelPtr->getTopic() << std::endl;
        result = CHANGE_TOPIC_MSG;
    }

	std::string topicMsg = channelPtr->channelMessage(result, this);
	std::cout << "topicmsg: " << topicMsg << std::endl;
	if (send(this->getClientFd(), topicMsg.c_str(), topicMsg.size(), 0) < 0)
	{
		std::cout << "setTopic: failed to send\r\n";
		close(this->getClientFd());
		return;
	}   

}