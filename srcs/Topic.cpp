#include "Client.hpp"
#include "utils.hpp"

void	Channel::sendNoTopic(Client* client)
{
	std::string	server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName();

	std::string topicMsg = makeNumericReply(server, RPL_NOTOPIC, nick, 
		{"#" + chanName}, "No topic is set");
	this->sendMsg(client, topicMsg);
}

void	Channel::sendTopicAndNames(Client* client)
{
	std::string	server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName();
	// send topic / no_topic
	if (this->getTopic().empty())
		this->sendNoTopic(client);
	else
		this->sendTopic(client);

	// send name list and end of list
	std::string nameReplyMsg = makeNumericReply(server, RPL_NAMREPLY, nick,  {"=", "#"+ chanName}, this->printUser() );
	
	this->sendMsg(client, nameReplyMsg);
	std::cout << "SENT NAMEPLY\n";
	
	std::string endOfNamesMsg = makeNumericReply(server, RPL_ENDOFNAMES, 
		nick, {"#" + chanName},	"End of /NAMES list.");
	this->sendMsg(client, endOfNamesMsg);
	std::cout << "SENT ENDOFNAMES\n";
}

void	Channel::sendTopic(Client* client)
{
	std::string	server = client->getServerName(),
				nick = client->getNick(),
				chanName = this->getChannelName();

	std::string topicMsg = makeNumericReply(server, RPL_TOPIC, nick, 
		{"#" + chanName}, this->getTopic());
	this->sendMsg(client, topicMsg);
	
	// sending topicwhotime
	std::time_t timestamp = this->getTopicTimestamp();
	std::string topicWhoMsg = makeNumericReply(server, RPL_TOPICWHOTIME,
		nick, {"#" + chanName, getTopicSetter()->getNick(), std::to_string(timestamp)}, "");
	this->sendMsg(client, topicWhoMsg);
}

bool Channel::canNonOpsSetTopic()
{
	auto it = _mode.find('t');
	if (it != _mode.end())
		return false;
	return true;
}

void Channel::sendOpPrivsNeededMsg(Client* client)
{
	std::string	server = client->getServerName(),
			nick = client->getNick(),
			chanName = this->getChannelName();

	std::string notOpsMsg = makeNumericReply(server, ERR_CHANOPRIVSNEEDED, 
		nick, {"#" + chanName}, "You're not channel operator");
	this->sendMsg(client, notOpsMsg);
}

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
			std::cout << "there is no channel saved in _joinedChannel" << std::endl;
		}
	}
	return nullptr;
}

void Channel::setTopic(std::string buffer, Client* client)
{
	// not test this yet
	// if (!this->canNonOpsSetTopic() && !client->isOps(this))
	// {
	// 	this->sendOpPrivsNeededMsg(client); 
	// 	return; 
	// }
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

	// client hasn't joined channel
	if (!channelPtr->isClientOnChannel(client))
	{
		channelPtr->sendClientErr(ERR_NOTONCHANNEL, &client);
		return;
	}

	if (tokens.size() == 1)
	{
		if (channelPtr && channelPtr->getTopic().empty())
			channelPtr->sendNoTopic(&client);
		else if (channelPtr && !channelPtr->getTopic().empty())
			channelPtr->sendTopic(&client);
	}
    else if (tokens.size() == 2)
    {
        // std::cout << "im here setting chan name: " << tokens[1] << std::endl;
        channelPtr->setTopic(tokens[1], &client);
        // std::cout << "im here sending chan name: " << tokens[1] << std::endl;
		channelPtr->channelMessage(CHANGE_TOPIC_MSG, &client);
    }
	else
	{
		std::cout << "too many params\n";
		return;
	}
}
