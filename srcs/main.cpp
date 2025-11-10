
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

int main(int argc, char *argv[])
{
	Server server;
	server.setupServerDetails(server, argc, argv);
	server.setupSocket();
	server.setupEpoll();
	while (true)
	{
		int eventCount = epoll_wait(server.getEpollfd(), server.getEpollEvents(), MAX_EVENTS, -1);
		for (int i = 0; i < eventCount; ++i)
		{
			if (server.getEpollEvents()[i].data.fd == server.getServerfd())
			{
				server.handleNewClient();
			}
			else
			{
				//This could be connect with already existing client...
				char buffer[1024] = {0};
				int clientFd = server.getEpollEvents()[i].data.fd;
				int clientIndex = 0;
				for (size_t i = 0; i < server.getClientInfo().size(); i++)
				{
					if (server.getClientInfo()[i].getClientFd() == clientFd)
					{
						clientIndex = i;
						break;
					}
				}
				server.getClientInfo()[clientIndex].recieve();
				/*
				if (recv(server.getClientInfo()[clientIndex].getClientFd(), buffer, sizeof(buffer), 0) <= 0)
					std::cout << "Did we encounter a problem" << std::endl;
				std::cout << "Message that we received : [" << buffer << "]" << std::endl;
				std::string evenBuffer(buffer);
				server.handleCommand(server, server.getClientInfo()[clientIndex], evenBuffer);
				*/
				//need to also deal with a situation if password is "empty string"
			}
		}
	}
}
