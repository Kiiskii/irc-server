#include "Client.hpp"

void Client::updateClientInfo(std::string bufferStr)
{
	(void) bufferStr;
	_clientNick = "trpham";
	_userName ="trpham";
	_hostName = "localhost";
	_serverName = "localhost";
}
