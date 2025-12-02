
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

int main(int argc, char *argv[])
{
	Server server;
	if (argc != 3)
	{
		std::cerr << INPUT_FORMAT << std::endl;
		exit (1);
	}
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
				//make into its own function?
				int clientFd = server.getEpollEvents()[i].data.fd;
				int clientIndex = 0;
				for (size_t i = 0; i < server.getClientInfo().size(); i++)
				{
					if (server.getClientInfo()[i]->getClientFd() == clientFd)
					{
						clientIndex = i;
						break;
					}
				}
				Client &c = *server.getClientInfo()[clientIndex];
				server.receive(c);
			}
		}
	}
}
