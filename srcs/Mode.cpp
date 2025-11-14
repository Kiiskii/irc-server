#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"


// MODE <#channel> [+/-modestring] [mode arguments]
static bool isValidModeCmd(std::string buffer)
{
	std::regex modeRegex("^MODE\\s+#[a-zA-Z_0-9]+\\s+([+-][a-zA-Z]+)+(\\s+[a-zA-Z_0-9]+)*$");
	if (std::regex_match(buffer, modeRegex))
		return true;
	std::cout << "DOES NOT match regex pattern for mode\n";
	return false;
}

static void extractModeAndParams(std::string buffer, std::string& modeStr, 
	std::string& args)
{
	// mode start position
	bool addMode = true, haveArgs = true;
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
	// all modes are extracted into modeStr
	modeStr = buffer.substr(modePos, modeEndPos - modePos);

	// extrat the args string
	if (haveArgs)
		args = buffer.substr(modeEndPos + 1);
	
}

void Channel::setMode(std::string buffer, Client* client)
{
	std::string					modeStr, args;
	std::vector<std::string>	argsVec;

	extractModeAndParams(buffer, modeStr, args);
	std::cout << "mode are: [" << modeStr << "]" << " and args [" << args << "]" << std::endl;

	if (!args.empty())
			argsVec = splitString(args, ' ');
	
	// this only handle the map of _mode, not yet what to response??
	std::string params, modeStatus;
	bool		addMode = true;
	channelMsg	msgEnum;

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
		}

		msgEnum = (this->*(_modeHandlers[mode]))(addMode, params);
		if (addMode)
			modeStatus = std::string(1, '+') + mode;
		else
			modeStatus =  std::string(1, '-') + mode;
	
		// not send back but broadcast to all user on channel
		// if cannot set a mode, what to do here?
		if (msgEnum == SET_MODE_OK)
			std::cout << "mode: set_mode_ok\n";
		std::cout << "mode: [" << mode << "] and params: [" << params << "]\n";

	}
	this->channelMessage(msgEnum, client, modeStr, args);
	// std::cout << "modeMsg: [" << modeMsg << "]" << std::endl;
	// for (auto user : this->getUserList())
	// {
	// 	if (send(user->getClientFd(), modeMsg.c_str(), modeMsg.size(), 0) < 0)
	// 	{
	// 		std::cout << "setMode: failed to send\r\n";
	// 		close(user->getClientFd());
	// 		return;
	// 	}
	// }
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

	// ONLY work on channel mode, so always have channel??
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
	// channelPtr->setMode(buffer, msgEnum, mode, params);
	channelPtr->setMode(buffer, this);
	channelPtr->getMode();
	// channelPtr->executeMode();

	// // not send back but broadcast to all user on channel
	// if (msgEnum == SET_MODE_OK)
	// 	std::cout << "mode: set_mode_ok\n";
	// std::cout << "mode: [" << mode << "] and params: [" << params << "]\n";

	// std::string modeMsg = channelPtr->channelMessage(msgEnum, this, mode, params);
	// std::cout << "modeMsg: [" << modeMsg << "]" << std::endl;
	// for (auto user : channelPtr->getUserList())
	// {
	// 	if (send(user.getClientFd(), modeMsg.c_str(), modeMsg.size(), 0) < 0)
	// 	{
	// 		std::cout << "setMode: failed to send\r\n";
	// 		close(user.getClientFd());
	// 		return;
	// 	}
	// }
}	
	
