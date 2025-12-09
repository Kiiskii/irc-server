#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"

static bool modeNeedParams(char mode, bool addMode)
{
	if (mode == I_MODE || mode == T_MODE) // no need params
		return false;
	if ( mode == L_MODE && addMode) // add needs param but remove NOT
		return true;
	else if (mode == L_MODE && !addMode)
		return false;
	if ( mode == K_MODE || mode == O_MODE) //both add and remove need params
		return true;
	return false;
}

bool Channel::parsingMode(Client& client, std::vector<std::string> tokens, std::vector<ModeInfo>& parsedModeVec)
{
	std::string modeStr;
	bool		addMode = true;
	while (!tokens.empty())
	{
		modeStr = tokens[0];
		tokens.erase(tokens.begin());

		for (char mode : modeStr)
		{
			if (mode == '+') {addMode = true; continue;}	
			if (mode == '-') {addMode = false; continue;}

			ModeInfo modeIn;
			modeIn.add = addMode;
			modeIn.mode = mode;

			if (modeNeedParams(mode, addMode))
			{
				if (tokens.empty()) // mode needs param but not given
					modeIn.params = "";
				else
				{
					modeIn.params = tokens.front();
					tokens.erase(tokens.begin());
				}
			}
			parsedModeVec.push_back(modeIn);
		}
	}

	return true;
}

/** @brief  If <modestring> is given, the user MUST have appropriate channel privileges */
bool Channel::validateModeInstruction(Client& client, std::vector<ModeInfo> parsedModeVec)
{
	Server& server = client.getServer();
	if (!this->isChanop(client.getNick()))
	{
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, client, this, {});
		return false;
	}

	for (auto m : parsedModeVec)
	{
		if (_modeHandlers.count(m.mode) == 0)
		{
			std::string unknownMode(1, m.mode);
			server.sendClientErr(ERR_UNKNOWNMODE, client, this, 
					{unknownMode});
			return false;
		}
		if (modeNeedParams(m.mode, m.add) && m.params.empty())
		{
			std::string cmd = "MODE ";
			cmd += m.add ? "+" : "-";
			cmd += m.mode;
			server.sendClientErr(461, client, this, {cmd});
			return false;
		}
		if (m.mode == O_MODE)
		{
			if (!server.findClient(m.params))
			{
				server.sendClientErr(ERR_NOSUCHNICK, client, this, {m.params});
				return false;
			}
			if (!this->isClientOnChannel(client))
			{
				server.sendClientErr(ERR_USERNOTINCHANNEL, client, this, {m.params});
				return false;
			}
		}
	}

	return true;
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
			// std::cout << "mode " << mode << std::endl;
			executedMode += addMode ? "+" : "-";
			executedMode += mode;
			// std::cout << "after adding mode " << executedMode << std::endl;
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

void Channel::executeModeCmd(Client& client, std::vector<ModeInfo>& parsedModeVec)
{
	channelMsg	msgEnum;
	std::string	executedMode, executedArgs;

	for (auto& m : parsedModeVec)
	{
		msgEnum = (this->*(_modeHandlers[m.mode]))(m.add, m.params);
		if (msgEnum == SET_MODE_OK)
		{
			std::cout << " set_mode_ok has mode: [" << m.mode << "] and params: [" << m.params << "]\n";
			combineExecutedMode(executedMode, m.mode, m.add);
			executedArgs += (m.params.empty() ? "" : m.params + " ");
		}
	}
	restrictRemoveKeyMode(executedMode, executedArgs);
	if (!executedMode.empty())
	{
		client.getServer().sendSetModeMsg(client, *this, executedMode, executedArgs);
	}
}

/** @brief mode applied: itkol => only handle mode for channel, not for user.*/
void Server::handleMode(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr = nullptr;
	if (tokens.empty())
	{
		sendClientErr(461, client, channelPtr, {"MODE"});
		return ;
	}

	std::string	channelName = tokens[0];
	if (!client.isValidChanName(channelName))
		return ;

	channelPtr = this->findChannel(utils::extractChannelName(channelName));
	if (!channelPtr) 
	{
		this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, 
			{utils::extractChannelName(channelName)});
		return ; 
	}

	tokens.erase(tokens.begin(), tokens.begin() + 1);

	//  If <modestring> is not given, inform currently-set modes of a channel. 
	if (tokens.empty())
	{
		std::cout << "mode str: " << channelPtr->getMode()[0] << " mode arg: " << channelPtr->getMode()[1];
		sendClientErr(RPL_CHANNELMODEIS, client, channelPtr, 
			{channelPtr->getMode()[0], channelPtr->getMode()[1]});
		// sendClientErr(RPL_CREATIONTIME, client, channelPtr, {}); //recheck this, irssi send always after join so duplicate??
		return ;
	}

	std::vector<ModeInfo> parsedModeVec;
	if (!channelPtr->parsingMode(client, tokens, parsedModeVec))
		return ;

	if (!channelPtr->validateModeInstruction(client, parsedModeVec))
		return ;

	channelPtr->executeModeCmd(client, parsedModeVec);
	
}
