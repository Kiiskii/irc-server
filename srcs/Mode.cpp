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

void Channel::setMode(std::string buffer, channelMsg& msgEnum, std::string& modeStatus, std::string& params)
{
	// mode start position
	bool addMode = true;
	bool haveArgs = true;
	size_t modePos = buffer.find("+");
	if ( modePos == std::string::npos)
	{
		addMode = false;
		modePos = buffer.find("-");
	}
	if (modePos == std::string::npos)
		return ;

	// mode end position
	size_t modeEndPos = buffer.find(' ', modePos);
	if (modeEndPos == std::string::npos)
	{
		modeEndPos = buffer.length();
		haveArgs = false;
	}
	// subtract the args string and split to vector
	std::string modeStr = buffer.substr(modePos, modeEndPos - modePos);
	std::string args;
	std::vector<std::string> argsVec;
	if (haveArgs)
	{
		args = buffer.substr(modeEndPos + 1);
		if (!args.empty())
			argsVec = splitString(args, ' ');
	}
	std::cout << "mode are: [" << modeStr << "]" << " and args [" << args << "]" << std::endl;

	// this only handle the map of _mode, not yet what to response??
	// std::string param;
	for (char mode : modeStr)
	{
		if (mode == '+') {addMode = true; continue;}	
		if (mode == '-') {addMode = false; continue;}
		if (mode == 'i' || mode == 't' || mode == 'o')
		{
			params = "";
		}
		else if (mode == 'k' || mode == 'l')
		{
			params = argsVec.front();
			argsVec.erase(argsVec.begin());
			// this->addMode(mode, argsVec.front());
		}
		std::cout << "param :[" << params << "] \n";
		msgEnum = (this->*(_modeHandlers[mode]))(addMode, params);
		if (addMode)
			modeStatus = std::string(1, '+') + mode;
		else
			modeStatus =  std::string(1, '-') + mode;
	}
}

void Channel::executeMode()
{

}

//mode: itkol
void	Client::changeMode(std::string buffer)
{
	Channel*	channelPtr = nullptr;
	channelMsg	msgEnum;
	// bool		addMode = true;
	std::string	params = "";

	// might validate the command here
	if (isValidModeCmd(buffer) == false)
	{
		std::cout << "Invalid mode cmd" << std::endl;
		return;
	}

	// need to divide handle channel and user separately?? now work on channel only
	if (buffer.find("#") != std::string::npos)
	{
		channelPtr = setActiveChannel(buffer);
		// if not on any channel, return do nothing
		if (channelPtr == nullptr)
		{
			std::cout << "null ptr \n";
			return;
		}
	}
	else 
	{
		std::cout << "message doesn't have channel # \n";
	}
	// std::cout << "here ok \n";

	std::string		mode;
	channelPtr->setMode(buffer, msgEnum, mode, params);
	channelPtr->getMode();
	// channelPtr->executeMode();

	// not send back but broadcast to all user on channel
	if (msgEnum == SET_MODE_OK)
		std::cout << "mode: set_mode_ok\n";
	std::cout << "mode: [" << mode << "] and params: [" << params << "]\n";

	std::string modeMsg = channelPtr->channelMessage(msgEnum, this, mode, params);
	std::cout << "modeMsg: [" << modeMsg << "]" << std::endl;
	for (auto user : channelPtr->getUserList())
	{
		if (send(user.getClientFd(), modeMsg.c_str(), modeMsg.size(), 0) < 0)
		{
			std::cout << "setMode: failed to send\r\n";
			close(user.getClientFd());
			return;
		}
	}
}	
	
