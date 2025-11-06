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

channelMsg checkTopicComd(std::string bufferStr, Client* currentClient, Channel* currentChan)
{
    (void)currentClient;
    // std::cout << "enter check comd :" << bufferStr << std::endl;
    // bufferStr = trimStr(bufferStr);
    // std::cout << "buffer :" << bufferStr << std::endl;
    if (bufferStr == "TOPIC" && currentChan->getTopic().empty())
        return NO_TOPIC_MSG;
    else if (bufferStr == "TOPIC" && !currentChan->getTopic().empty())
        return CHANNEL_TOPIC_MSG;
    else if (bufferStr.find(":") != std::string::npos)
    {
        currentChan->setTopic(bufferStr);
        std::cout << "topic after set: " << currentChan->getTopic() << std::endl;
        return CHANGE_TOPIC_MSG;
    }
    return NO_MSG;
}
