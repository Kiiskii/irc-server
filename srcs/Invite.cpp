#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

/** @brief all the client has been invited to join by chanop */
bool	Channel::hasInvitedClient(Client* client)
{
	if (!_invitedUser.empty() && _invitedUser.find(client) != _invitedUser.end())
		return true;
	return false;
}

static bool isValidInvitation(std::vector<std::string>& tokens, Client& client)
{
	if (tokens.size() < 2)
	{
		std::string msg = ERR_NEEDMOREPARAMS(client.getServer().getServerName(), client.getNick(), "INVITE");
		client.getServer().sendMsg(client, msg);
		return false;
	}
	std::string nickName = tokens[0];
	std::string chanName = tokens[1];
	
	if (!client.getServer().findChannel(chanName))
		



	return true;
}

/** @brief The INVITE command is used to invite a user to a channel. The parameter 
 * <nickname> is the nickname of the person to be invited to the target channel <channel>.
 * The target channel SHOULD exist (at least one user is on it). Otherwise, the server
 * SHOULD reject the command with the ERR_NOSUCHCHANNEL numeric.
 * Only members of the channel are allowed to invite other users. Otherwise, the server
 *  MUST reject the command with the ERR_NOTONCHANNEL numeric.
 * Servers MAY reject the command with the ERR_CHANOPRIVSNEEDED numeric. In particular, they SHOULD reject it when the channel has invite-only mode set, and the user is not a channel operator.
 */
void Server::handleInvite(Client& client, std::vector<std::string> tokens)
{


	if (!isValidInvitation(tokens, client))
		return;

	

}


