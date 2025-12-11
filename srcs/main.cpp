
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

extern bool activeserver = true;

void signalHandler(int sig)
{
	activeserver = false;
}

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
	signal(SIGINT, signalHandler);
	while (activeserver == true)
	{
		server.handleEvents();
	}
}
