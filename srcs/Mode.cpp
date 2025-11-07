#include "Client.hpp"
#include "utils.hpp"


// MODE <#channel> [+/-modestring] [mode arguments]
static bool isValidModeCmd(std::string buffer)
{
	std::regex modeRegex("^MODE\\s+#[a-zA-Z_0-9]+\\s+[+-][a-zA-Z]+(?:\\s+[a-zA-Z_0-9]+)?$");
	if (std::regex_match(buffer, modeRegex))
	{
		std::cout << "match regex pattern for mode\n";
		return true;
	}
	return false;
}

//  MODE #mama +k hihi
// MODE #mama -o trang
//mode: itkol
void	Client::changeMode(std::string buffer)
{
	Channel* channelPtr;
	channelMsg result;

	// std::cout << "channel size: " << this->_joinedChannels.size() << std::endl;

	// might validate the command here
	if (isValidModeCmd(buffer) == false)
		return;

	// need to divide handle channel and user separately??
	// if (buffer.find("#") != std::string::npos)
	// {
	// 	channelPtr = setActiveChannel(buffer);
	// 	// if not on any channel, return do nothing
	// 	if (channelPtr == nullptr)
	// 		return;
	// 	std::cout << "here " <<  std::endl;


	// 	std::map<char, void (Channel::*)(bool, std::string&)> modeHandlers = {
	// 			std::make_pair('i', &Channel::handleInviteOnly), // channel
	// 			std::make_pair('t', &Channel::handleTopicRestriction), // user
	// 			std::make_pair('k', &Channel::handleChannelKey), //channel
	// 			std::make_pair('o', &Channel::handleChannelOperator), // user
	// 			std::make_pair('l', &Channel::handleChannelLimit) // channel
	// 	};

	// }

	bool addMode = false;
	size_t modePos;
	if (buffer.find("+") != std::string::npos)
	{
		addMode = true;
		modePos = buffer.find_first_of("+");
	}
	else
		modePos = buffer.find_first_of("-");
	
	buffer = buffer.erase(0, modePos);
	std::cout << "mode + argument: " << buffer << std::endl;


	// if (buffer == "TOPIC")
	// {
	// 	if (channelPtr && channelPtr->getTopic().empty())
	// 		result = NO_TOPIC_MSG;
	// 	else if (channelPtr && !channelPtr->getTopic().empty())
	// 		result = CHANNEL_TOPIC_MSG;
	// }
    // else if (buffer.find(":") != std::string::npos)
    // {
    //     // std::cout << "im here setting chan name: " << std::endl;
    //     channelPtr->setTopic(buffer);
    //     std::cout << "topic after set: " << channelPtr->getTopic() << std::endl;
    //     result = CHANGE_TOPIC_MSG;
    // }

	// std::string topicMsg = channelPtr->channelMessage(result, this);
	// std::cout << "topicmsg: " << topicMsg << std::endl;
	// if (send(this->getClientFd(), topicMsg.c_str(), topicMsg.size(), 0) < 0)
	// {
	// 	std::cout << "setTopic: failed to send\r\n";
	// 	close(this->getClientFd());
	// 	return;
	// }
}	
	
