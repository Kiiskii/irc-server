#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"


/** @note this check only for mode applied to channel, not sure about users mode
 * @brief MODE <#channel> <+/-modestring> [<mode arguments>...] */ 
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
			std::string str(1, static_cast<char>(modeStr[i])); //check this chararacter
			client.getServer().sendClientErr(ERR_UNKNOWNMODE, client, this, {str});
			return false;
		}
	}
	std::cout << "DOES NOT match regex pattern for mode\n";
	client.getServer().sendClientErr(ERR_UNKNOWNMODE, client, this, {});
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
	Server&	server = client.getServer();

	std::cout << "mode are: [" << modeStr << "]" << " and args. ";
	printVector(argsVec);

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
			std::cout << "size of arg" << argsVec.size() << std::endl;
			params = argsVec.front();
			argsVec.erase(argsVec.begin());
		}
		else
			server.sendClientErr(ERR_UNKNOWNMODE, client, this, {});
		// this only handle the _mode mapping, not action with client yet
		msgEnum = (this->*(_modeHandlers[mode]))(addMode, params);
		if (msgEnum == SET_MODE_OK)
		{
			std::cout << " set_mode_ok has mode: [" << mode << "] and params: [" << params << "]\n";
			combineExecutedMode(executedMode, mode, addMode);
			executedArgs += (params.empty() ? "" : params + " ");
		}
		// if cannot set a mode, what to do here?

	}
	restrictRemoveKeyMode(executedMode, executedArgs);
	server.channelMessage(msgEnum, &client, this, executedMode, executedArgs);
}

/** @brief mode applied: itkol */
void Server::handleMode(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr = nullptr;
	std::string	nameStr, modeStr;
	std::vector<std::string> modeParams;

	if (tokens.size() > 0)
		nameStr = tokens[0];
	if (tokens.size() > 1)
		modeStr = tokens[1];
	if (tokens.size() > 2)
	{
		tokens.erase(tokens.begin(), tokens.begin() + 2);
		modeParams = tokens;
	}

	// ONLY work on channel mode, so always have channel??
	if (nameStr.find("#") != std::string::npos)
	{
		channelPtr = this->setActiveChannel(nameStr);
		// if not on any channel, return do nothing
		if (channelPtr == nullptr) 
		{
			std::cout << "null ptr \n";	return; 
		}
	}
	else 
	{
		// std::cout << "message doesn't have channel # \n"; 
		return;
	}

	// validate the command here, need to fix this??
	if (channelPtr->isValidModeCmd(modeStr, client) == false)
		return;

	channelPtr->setMode(modeStr, modeParams, client);
	channelPtr->getMode(); //=> to print the mode active, remove later
	// std::cout << "print mode ok: " << std::endl;
	channelPtr->getOps();
}	
