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

	if (quitMsg.empty())
		quitMsg = "Quit: Client Quit";
	else
		quitMsg = "Quit: " + quitMsg;
	client.setQuitMsg(quitMsg);
	// std::string serverMsg = client.makeUser() + " QUIT :" 
	// 	+ (quitMsg.empty() ? ("Quit: Client Quit") : "Quit: " + quitMsg) + "\r\n";

	// this->broadcastUsersMsg(serverMsg, client, false);
	client.setClientState(DISCONNECTING);
	
	return;
}
void	Client::setQuitMsg(std::string msg)
{
	_quitMsg = msg;
}
