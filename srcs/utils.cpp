#include "utils.hpp"

std::string ft_trimString(std::string msg)
{
    std::string leadingTrim = msg.substr(msg.find_first_not_of(" \a\b\t\n\\v\f\r"), msg.length() - msg.find_first_not_of(" \a\b\t\n\\v\f\r"));
    std::string trailingTrim = leadingTrim.substr(0, leadingTrim.find_last_not_of(" \a\b\t\n\\v\f\r") + 1);
    return trailingTrim;
}


/** @brief split string into tokens using delimiter */
std::vector<std::string> splitString(std::string buffer, char delimiter)
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
		+ (p.empty() ? "" : " " + p)
		+ (trailing.empty() ? "" : ":" + trailing)
		+ "\r\n";
	return s;
}