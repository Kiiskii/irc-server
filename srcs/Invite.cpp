#include "Channel.hpp"
#include "Client.hpp"

/** @brief all the client has been invited to join by chanop */
bool	Channel::hasInvitedClient(Client* client)
{
	if (!_invitedUser.empty() && _invitedUser.find(client) != _invitedUser.end())
		return true;
	return false;
}
