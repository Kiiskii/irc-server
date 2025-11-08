#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"


// MODE <#channel> [+/-modestring] [mode arguments]
static bool isValidModeCmd(std::string buffer)
{
	std::regex modeRegex("^MODE\\s+#[a-zA-Z_0-9]+\\s+[+-][a-zA-Z]+(?:\\s+[a-zA-Z_0-9]+)?$");
	if (std::regex_match(buffer, modeRegex))
		return true;
	std::cout << "DOES NOT match regex pattern for mode\n";
	return false;
}

void Channel::setMode(std::string buffer)
{
	bool addMode = true;
	bool haveArgs = true;
	size_t modePos = buffer.find("+");
	if ( modePos == std::string::npos)
	{
		addMode = false;
		modePos = buffer.find("-");
	}
	size_t modeEndPos = buffer.find(' ', modePos);
	if (modeEndPos == std::string::npos)
	{
		modeEndPos = buffer.length();
		haveArgs = false;
	}

	std::string modeStr = buffer.substr(modePos + 1, modeEndPos - modePos - 1);
	std::string args;
	std::vector<std::string> argsVec;
	if (haveArgs)
	{
		args = buffer.substr(modeEndPos + 1);
		argsVec = splitString(args, ' ');
	}
	// std::cout << "mode are: [" << modeStr << "]" << " and args [" << args << "]" << std::endl;

	// need to check whether the mode exist, if add then add, if remove then remove, else if need to change then change param 
	// how to include the function pointer to here
	for (size_t i = 0; i < modeStr.size(); ++i)
	{
		if (modeStr[i] == '+')
			addMode = true;
		else if (modeStr[i] == '-')
			addMode = false;
		else if (modeStr[i] == 'i' || modeStr[i] == 't' || modeStr[i] == 'o')
			this->addMode(modeStr[i], "");
		else
		{
			this->addMode(modeStr[i], argsVec.front());
			argsVec.erase(argsVec.begin());
		}
	}
	
}

void Channel::executeMode()
{

}


//  MODE #mama +k hihi
// MODE #mama -o trang
//mode: itkol
void	Client::changeMode(std::string buffer)
{
	Channel* channelPtr;
	channelMsg result;

	// might validate the command here
	if (isValidModeCmd(buffer) == false)
		return;

	// need to divide handle channel and user separately??
	if (buffer.find("#") != std::string::npos)
	{
		channelPtr = setActiveChannel(buffer);
		// if not on any channel, return do nothing
		if (channelPtr == nullptr)
		{
			std::cout << "null ptr \n";
			return;
		}
		std::map<char, void (Channel::*)(bool, std::string&)> modeHandlers = {
				std::make_pair('i', &Channel::handleInviteOnly), // channel
				std::make_pair('t', &Channel::handleTopicRestriction), // user
				std::make_pair('k', &Channel::handleChannelKey), //channel
				std::make_pair('o', &Channel::handleChannelOperator), // user
				std::make_pair('l', &Channel::handleChannelLimit) // channel
		};
	}

	channelPtr->setMode(buffer);
	channelPtr->getMode();
	channelPtr->executeMode();


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
	
