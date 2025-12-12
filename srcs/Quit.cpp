#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

using namespace utils;

void Server::handleQuit(Client& client, std::vector<std::string>& tokens)
{
	std::string quitMsg = utils::joinTokenVector(tokens);
	if (quitMsg == ":")
		quitMsg = "";
	else if (quitMsg.find_first_not_of(':') != std::string::npos)
		quitMsg = quitMsg.substr(quitMsg.find_first_not_of(':'));

	std::string serverMsg = client.makeUser() + " QUIT :" 
		+ (quitMsg.empty() ? ("Quit: Client Quit") : "Quit: " + quitMsg) + "\r\n";

	std::unordered_set<Client*> quitMsgClient{};
	for (auto chan : client.getJoinedChannels())
	{
		for (auto user : chan->getUserList())
			quitMsgClient.insert(user);
	}
	for (auto user : quitMsgClient)
	{
		if (user != &client)
			sendMsg(*user, serverMsg);
	}
	client.setClientState(DISCONNECTING);
	return;
}
