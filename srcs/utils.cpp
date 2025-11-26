#include "utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"

using namespace utils;

/** @brief split string into tokens using delimiter */
std::vector<std::string> utils::splitString(std::string buffer, char delimiter)
{
	std::cout << "buffer :[" << buffer << "]" << std::endl;

	std::istringstream	tokenStream(buffer); //save buffer string to an istringstream obj
	std::string			aToken;
	std::vector<std::string> tokens;

	while (std::getline(tokenStream, aToken, delimiter))
	{
		tokens.push_back(aToken);
	}
	return tokens;
}

/* The most common form of reply is the numeric reply, used for both errors and normal replies. Distinct from a normal message, a numeric reply MUST contain a <source> and use a three-digit numeric as the command. A numeric reply SHOULD contain the target of the reply as the first parameter of the message. A numeric reply is not allowed to originate from a client */
// : channel names, usernames, modes, numbers -> param
// Everything after : is considered the trailing field
std::string makeNumericReply(std::string prefix, int code, std::string target, std::vector<std::string> params, std::string trailing)
{
	std::string p, s;
	for (auto param : params)
		p += param + " ";
	s = ":" + prefix + " " + std::to_string(code) + " " + target 
		+ (p.empty() ? " " : " " + p)
		+ (trailing.empty() ? "" : ":" + trailing)
		+ "\r\n";
	// std::cout << ": " << s << std::endl;
	return s;
}

/** @note not consider the case of local channel start with '&' ?? 
 * case insensitive ??
*/
bool	utils::isValidChanName(std::string name)
{
	std::regex chanNameRegex("^#[^ \\x07,]+$");

	if (!std::regex_match(name, chanNameRegex))
	{
		std::cout << "Bad channel names\n";
		return false;
	}
	return true;
}

void	utils::printVector(std::vector<std::string> tokens)
{
	std::cout << "vector memebers: " << std::endl;

	if (tokens.empty())
	{
		std::cout << "empty tokens\n";
		return;
	}
	for (auto token : tokens)
	{
		std::cout << "[" <<  token << "] ";
	}
	std::cout << std::endl;
}

std::string getTarget(Client &client)
{
	std::string target;
	if (client.getNick().empty())
	{
		target = "*";
	}
	else
		target = client.getNick();
	return target;
}

void	utils::printOps(Channel& channel)
{
	std::cout << "@CHANOPS list: \n";
	if (!channel.getOps().empty())
	{
		for (auto op : channel.getOps())
		{
			std::cout << op->getNick() << ", ";
		}
		std::cout << "\n";
	}
}