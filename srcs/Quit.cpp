#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

using namespace utils;

void Server::handleQuit(Client& client, std::vector<std::string>& tokens)
{
	std::string quitMsg = utils::joinTokenVector(tokens);
	if (quitMsg.find_first_not_of(':') != std::string::npos)
		quitMsg = quitMsg.substr(quitMsg.find_first_not_of(':'));
	// if (quitMsg == "leaving")
	// 	quitMsg = "";
	// std::cout << "quitmsg : [" << quitMsg << "]\n";

	std::string serverMsg = client.makeUser() + " QUIT :" 
		+ (quitMsg.empty() ? ("Quit: ") : "Quit: " + quitMsg) + "\r\n";
	std::cout << "servermsg : [" << serverMsg << "]\n";


	for (auto chan : client.getJoinedChannels())
	{
		this->broadcastChannelMsg(serverMsg, *chan, client);
		chan->removeUser(client.getNick());
	}
	// this->disconnectClient(&client); //already called in destructor
	client.setClientState(DISCONNECTING);
	return;

}
