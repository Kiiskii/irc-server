#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"



Channel* Client::setActiveChannel(std::string buffer)
{
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
			std::string server = this->_myServer.getServerName(),
				nick = this->getNick();
	
			std::cout << "there is no channel saved in _joinedChannel" << std::endl;
			std::string msg = makeNumericReply(server, ERR_NOTONCHANNEL, nick, {"#" + channelName}, "You're not on that channel");
			if (send(this->getClientFd(), msg.c_str(), msg.size(), 0) < 0)
			{
				std::cout << "joinmsg: failed to send\n";
				return nullptr;
			}
		}
	}
	return nullptr;
}

/** @brief if the t_mode is on, only chanop can set/remove topic */
void Channel::setTopic(std::string buffer, Client& client, Server& server)
{
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

	channelPtr = client.setActiveChannel(tokens[0]);
	// if not on any channel, return do nothing
	if (channelPtr == nullptr)
		return;

	// // client hasn't joined channel
	// if (!channelPtr->isClientOnChannel(client))
	// {
	// 	channelPtr->sendClientErr(ERR_NOTONCHANNEL, &client);
	// 	return;
	// }

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
        channelPtr->setTopic(tokens[1], client, *this);
        // std::cout << "im here sending chan name: " << tokens[1] << std::endl;
		this->channelMessage(CHANGE_TOPIC_MSG, &client, channelPtr);
    }
	else
	{
		std::cout << "too many params\n";
		return;
	}
}
