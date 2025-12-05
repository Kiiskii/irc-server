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

static void combineExecutedMode(std::string& executedMode, char& mode, bool addMode)
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
		{
			std::cout << "mode " << mode << std::endl;
			executedMode += addMode ? "+" : "-";
			executedMode += mode;
			std::cout << "after adding mode " << executedMode << std::endl;
		}
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

/** @brief  If <modestring> is given, the user MUST have appropriate channel privileges */
bool Channel::setMode(std::vector<std::string>& tokens, Client& client)
{
	Server&		server = client.getServer();
	std::string modeStr, params, executedMode, executedArgs;
	bool		addMode = true;
	channelMsg	msgEnum;

	if (!client.isOps(*this))
	{
		client.getServer().sendClientErr(ERR_CHANOPRIVSNEEDED, client, this, {});
		return false;
	}

	while (!tokens.empty())
	{
		modeStr = tokens[0];
		tokens.erase(tokens.begin());

		for (char mode : modeStr)
		{
			if (mode == '+') {addMode = true; continue;}	
			if (mode == '-') {addMode = false; continue;}
			if (mode == 'i' || mode == 't') // no need params
				params = "";
			else if ( mode == 'l') // add needs param but remove NOT
			{
				if (addMode)
				{
					if (tokens.empty())
					{
						std::string cmd = "MODE ";
						cmd += addMode ? "+" : "-";
						cmd += mode;
						server.sendClientErr(461, client, this, {cmd});
						continue;
					}
					else
						params = utils::setParamAndRemoveToken(tokens);
				}
				else
					params = "";
			}
			else if ( mode == 'k') //both add and remove need params
			{
				if (tokens.empty())
				{
					std::string cmd = "MODE ";
					cmd += addMode ? "+" : "-";
					cmd += mode;
					server.sendClientErr(461, client, this, {cmd});
					continue;
				}
				else
					params = utils::setParamAndRemoveToken(tokens);
			}
			else if (mode == 'o')
			{
				if (tokens.empty())
				{
					std::string cmd = "MODE ";
					cmd += addMode ? "+" : "-";
					cmd += mode;
					server.sendClientErr(461, client, this, {cmd});
					continue;
				}
				else
					params = utils::setParamAndRemoveToken(tokens);
			}
			else
			{
				std::string unknowMode(1, mode);
				client.getServer().sendClientErr(ERR_UNKNOWNMODE, client, this, 
					{unknowMode});
				continue; //return and set no mode at all, even the valid one
			}

			msgEnum = (this->*(_modeHandlers[mode]))(addMode, params);
			if (msgEnum == SET_MODE_OK)
			{
				// std::cout << " set_mode_ok has mode: [" << mode << "] and params: [" << params << "]\n";
				combineExecutedMode(executedMode, mode, addMode);
				executedArgs += (params.empty() ? "" : params + " ");
			}
		}
	}
	restrictRemoveKeyMode(executedMode, executedArgs);
	if (!executedMode.empty())
	{
		server.sendSetModeMsg(client, *this, executedMode, executedArgs);
	}
	return true;
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

/** @brief mode applied: itkol => only handle mode for channel, not for user.
 * If <modestring> is not given, inform currently-set modes of a channel. */
void Server::handleMode(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr = nullptr;

	if (tokens.empty())
	{
		sendClientErr(461, client, channelPtr, {"MODE"});
		return;
	}

	std::string	channelName = tokens[0];
	if (!client.isValidChanName(channelName))
		return;

	channelPtr = this->findChannel(utils::extractChannelName(channelName));
	if (!channelPtr) 
	{
		this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, 
			{utils::extractChannelName(channelName)});
		return; 
	}

	tokens.erase(tokens.begin(), tokens.begin() + 1);
	if (tokens.empty())
	{
		sendClientErr(RPL_CHANNELMODEIS, client, channelPtr, 
			{channelPtr->getMode()[0], channelPtr->getMode()[1]});
		sendClientErr(RPL_CREATIONTIME, client, channelPtr, {}); //recheck this, irssi send always after join so duplicate??
		return;
	}

	if (!channelPtr->setMode(tokens, client))
		return;
}
