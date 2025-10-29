#include "Client.hpp"

/* need to fix this one, currently fix value for channel testing */
void Client::updateClientInfo(std::string bufferStr)
{
	(void) bufferStr;
	_clientNick = "trpham";
	_userName ="trpham";
	_hostName = "localhost";
	_serverName = "localhost";
}
