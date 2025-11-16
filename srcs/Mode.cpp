#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"


/** @note this check only for mode applied to channel, not sure about users mode
 * @brief MODE <#channel> <+/-modestring> [<mode arguments>...] */ 
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

	// extract the args string
	if (haveArgs)
		args = buffer.substr(modeEndPos + 1);
	
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

	if (executedMode.empty())
		return;
	for (size_t i  = 0; i < executedMode.length(); i++)
	{
		std::cout << "value at i : " << executedMode[i] << std::endl;
		if (executedMode[i] == '+')
			activeAddMode = true;
		else if (executedMode[i] == '-')
			activeAddMode = false;
		else if (executedMode[i] == 'k')
			break;
	}
	if (activeAddMode == false)
		executedArgs = "*";
}

void Channel::setMode(std::string buffer, Client* client)
{
	std::string					modeStr, args;
	std::vector<std::string>	argsVec;

	extractModeAndParams(buffer, modeStr, args);
	// std::cout << "mode are: [" << modeStr << "]" << " and args [" << args << "]" << std::endl;

	if (!args.empty())
			argsVec = splitString(args, ' ');
	
	std::string params, modeStatus, executedMode, executedArgs;
	bool		addMode = true;
	channelMsg	msgEnum;

	for (char mode : modeStr)
	{
		if (mode == '+') {addMode = true; continue;}	
		if (mode == '-') {addMode = false; continue;}
		if (mode == 'i' || mode == 't')
			params = "";
		else if ( mode == 'l' || mode == 'o')
		{
			if (argsVec.empty())
			{
				sendClientErr(461, client);
				break;
			}	
			params = argsVec.front();
			argsVec.erase(argsVec.begin());
		}
		else if (mode == 'k')
		{
			if (argsVec.empty()) 
				params = "";
			else
			{
				params = argsVec.front();
				argsVec.erase(argsVec.begin());
			}
		}

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
	this->channelMessage(msgEnum, client, executedMode, executedArgs);
}

/** @brief mode applied: itkol */
void	Client::changeMode(std::string buffer)
{
	Channel*	channelPtr = nullptr;

	// validate the command here
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
		if (channelPtr == nullptr) {
			std::cout << "null ptr \n";	return; }
	}
	else {
		std::cout << "message doesn't have channel # \n";
	}

	std::string		mode;
	channelPtr->setMode(buffer, this);
	// channelPtr->getMode(); //=> to print the mode active
}	
	
/**	@brief if this mode is set on a channel, a user must have received an INVITE for this channel before being allowed to join it. If they have not received an invite, they will receive an ERR_INVITEONLYCHAN (473) reply and the command will fail. --> when to handle client ?? */
channelMsg Channel::handleInviteOnly(bool add, std::string& args)
{
	bool active = false;

	for (auto m : _mode)
	{
		if (m.first == 'i')
		{
			active = true;
			break;
		}
	}
	if (add)
	{
		if (active)
			return NO_ACTION;
		this->addMode('i', args);
		return SET_MODE_OK;
	}
	else
	{
		if (active)
		{
			this->removeMode('i');
			return SET_MODE_OK;
		}
	}
	return NO_ACTION;
}

channelMsg	Channel::handleTopicRestriction(bool add, std::string& args)
{

	return SET_MODE_OK;

}

/**
 * @brief This mode letter sets a ‘key’ that must be supplied in order to join this channel. If this mode is set, its’ value is the key that is required. Servers may validate the value (eg. to forbid spaces, as they make it harder to use the key in JOIN messages). If the value is invalid, they SHOULD return ERR_INVALIDMODEPARAM. However, clients MUST be able to handle any of the following:

    ERR_INVALIDMODEPARAM
    ERR_INVALIDKEY
    MODE echoed with a different key (eg. truncated or stripped of invalid characters)
    the key changed ignored, and no MODE echoed if no other mode change was valid.

If this mode is set on a channel, and a client sends a JOIN request for that channel, they must supply <key> in order for the command to succeed. If they do not supply a <key>, or the key they supply does not match the value of this mode, they will receive an ERR_BADCHANNELKEY (475) reply and the command will fail. */
channelMsg	Channel::handleChannelKey(bool add, std::string& args)
{
	bool active = false;
	std::string key;
	for (auto m : _mode)
	{
		if (m.first == 'k')
		{
			active = true;
			key = m.second;
			break;
		}
	}
	if (add)
	{
		if (active && args == key)
			return NO_ACTION;
		this->removeMode('k');
		this->addMode('k', args);
		return SET_MODE_OK;
	}
	else
	{
		if (active)
		{
			this->removeMode('k');
			return SET_MODE_OK; //recheck, send an empty key or nothing
		}
	}
	return NO_ACTION;
}

channelMsg	Channel::handleChannelOperator(bool add, std::string& args)
{
	return SET_MODE_OK;
}

channelMsg	Channel::handleChannelLimit(bool add, std::string& args)
{
	return SET_MODE_OK;

}