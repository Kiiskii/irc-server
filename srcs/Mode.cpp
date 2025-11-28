#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"

using namespace utils;

/** 
 * @brief MODE <#channel> <+/-modestring> [<mode arguments>...]
 * this function checks whether all the modes are valid 
 * and return which mode is not valid*/ 
bool Channel::isValidModeCmd(std::string modeStr, Client& client)
{
	std::regex modeRegex("^[+-][iklot]+([+-][iklot]+)*$");
	if (std::regex_match(modeStr, modeRegex))
		return true;

	std::string validChars = "+-iklot";
	for (size_t i = 0; i < modeStr.length(); ++i)
	{
		if (validChars.find(modeStr[i]) == std::string::npos)
		{
			std::string unknowMode(1, modeStr[i]);
			client.getServer().sendClientErr(ERR_UNKNOWNMODE, client, this, {unknowMode});
			return false;
		}
	}
	return false;
}

static void combineExecutedMode(std::string& executedMode, char mode, bool addMode)
{
	bool activeAddMode = true;

	if (executedMode.empty())
	{
		if (addMode)
			executedMode += "+";
		else
			executedMode += "-";
		executedMode += mode;
	}
	else
	{
		for (size_t i  = 0; i < executedMode.length(); i++)
		{
			if (executedMode[i] == '+')
				activeAddMode = true;
			else if (executedMode[i] == '-')
				activeAddMode = false;
		}
		if (addMode == activeAddMode)
			executedMode += mode;
		else
			executedMode += (addMode ? "+" : "-") + mode;
	}
}

/** @brief Servers MAY choose to hide sensitive information when sending the mode changes like key MODE args. Use asterisk to hide that args*/
static void restrictRemoveKeyMode(std::string& executedMode, std::string& executedArgs)
{
	bool activeAddMode;
	bool hasKeyMode = false;

	if (executedMode.empty())
		return;
	for (size_t i  = 0; i < executedMode.length(); i++)
	{
		// std::cout << "value at i : " << executedMode[i] << std::endl;
		if (executedMode[i] == '+')
			activeAddMode = true;
		else if (executedMode[i] == '-')
			activeAddMode = false;
		else if (executedMode[i] == 'k')
		{
			hasKeyMode = true;
			break;
		}
	}
	if (activeAddMode == false && hasKeyMode)
		executedArgs = "*";
}

void Channel::setMode(std::string& modeStr, std::vector<std::string> argsVec, Client& client)
{
	Server&		server = client.getServer();
	std::string params, modeStatus, executedMode, executedArgs;
	bool		addMode = true;
	channelMsg	msgEnum;

	for (char mode : modeStr)
	{
		if (mode == '+') {addMode = true; continue;}	
		if (mode == '-') {addMode = false; continue;}
		if (mode == 'i' || mode == 't')
			params = "";
		else if ( mode == 'l')
		{
			if (addMode && argsVec.empty())
			{
				server.sendClientErr(461, client, this, {});
				break;
			}
			if (!argsVec.empty() && addMode)
			{
				params = argsVec.front();
				argsVec.erase(argsVec.begin());
			}
		}
		else if ( mode == 'k')
		{
			if (addMode && argsVec.empty())
			{
				server.sendClientErr(461, client, this, {});
				break;
			}
			if (!argsVec.empty() && mode == 'k')
			{
				params = argsVec.front();
				argsVec.erase(argsVec.begin());
			}
		}
		else if (mode == 'o')
		{
			if (argsVec.empty()) 
			{
				server.sendClientErr(461, client, this, {});
				break;
			}
			// std::cout << "size of arg" << argsVec.size() << std::endl;
			params = argsVec.front();
			argsVec.erase(argsVec.begin());
		}
		else
		{
			std::string unknowMode(1, mode);
			// printf("%s", unknowMode.c_str());
			client.getServer().sendClientErr(ERR_UNKNOWNMODE, client, this, {unknowMode});
			return ;
			// server.sendClientErr(ERR_UNKNOWNMODE, client, this, {});
			// continue;
		}
		// this only handle the _mode mapping, not action with client yet
		msgEnum = (this->*(_modeHandlers[mode]))(addMode, params);
		if (msgEnum == SET_MODE_OK)
		{
			std::cout << " set_mode_ok has mode: [" << mode << "] and params: [" << params << "]\n";
			combineExecutedMode(executedMode, mode, addMode);
			executedArgs += (params.empty() ? "" : params + " ");
		}
		std::cout << " executedMode: [" << mode << "] and executedArgs: [" << params << "]\n";
	}
	restrictRemoveKeyMode(executedMode, executedArgs);
	server.sendSetModeMsg(client, *this, executedMode, executedArgs);
}

void Server::sendSetModeMsg(Client& client, Channel& channel, std::string& executedMode, std::string& executedArgs)
{
	std::string modeStr;
	if (!executedMode.empty())
		modeStr += executedMode;
	if (!executedArgs.empty())
		modeStr += " " + executedArgs;
	std::string	modeMsg = client.makeUser() + " MODE #" + 
			channel.getChannelName() + " " + modeStr + " \r\n";
	this->broadcastChannelMsg(modeMsg, channel);
}

/** @brief mode applied: itkol */
void Server::handleMode(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr = nullptr;
	std::string	nameStr, modeStr;
	std::vector<std::string> modeParams;

	if (tokens.size() < 2)
		return;
	nameStr = tokens[0];
	modeStr = tokens[1];
	if (tokens.size() > 2)
	{
		tokens.erase(tokens.begin(), tokens.begin() + 2);
		modeParams = tokens;
	}

	// set channel
	if (nameStr.find("#") != std::string::npos)
	{
		channelPtr = this->findChannel(utils::extractChannelName(nameStr));
		if (!channelPtr) 
		{
			this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, 
				{utils::extractChannelName(nameStr)});
			return; 
		}
	}
	else 
	{
		// this->sendClientErr(ERR_NOTONCHANNEL, client, nullptr, {});
		return;
	}

	std::cout << "name str: " << channelPtr->getChannelName() << " and modestr " << modeStr << "\nparams are: " ; 
	utils::printVector(tokens);

	

	if (!channelPtr->isValidModeCmd(modeStr, client))
		return;

	if (!client.isOps(*channelPtr))
	{
		client.getServer().sendClientErr(ERR_CHANOPRIVSNEEDED, client, channelPtr, {});
		return ;
	}

	channelPtr->setMode(modeStr, modeParams, client);
	// channelPtr->getMode(); //=> to print the mode active, remove later
	// std::cout << "print mode ok: " << std::endl;
	// channelPtr->getOps();
}	
