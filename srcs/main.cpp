
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
		int events = epoll_wait(server.epollfd, server.events, MAX_EVENTS, -1);
		for (int i = 0; i < events; ++i)
		{
			if (server.events[i].data.fd == server.serverfd)
			{
				server.handleNewClient();
			}
			else
			{
				//This could be connect with already existing client...
				char buffer[1024] = {0};
				int clientFd = server.events[i].data.fd;
				int clientIndex = 0;
				for (size_t i = 0; i < server.clientInfo.size(); i++)
				{
					if (server.clientInfo[i].getClientFd() == clientFd)
					{
						clientIndex = i;
						break;
					}
				}
				if (recv(server.clientInfo[clientIndex].getClientFd(), buffer, sizeof(buffer), 0) <= 0)
					std::cout << "Did we encounter a problem" << std::endl;
				std::cout << "Message that we received : [" << buffer << "]" << std::endl;
				std::string evenBuffer(buffer);
				server.handleCommand(server, server.clientInfo[clientIndex], evenBuffer);
				//need to also deal with a situation if password is "empty string"

				// if (evenBuffer.find("TOPIC ") != std::string::npos)
				// {
				// 	Client& currentClient =  server.clientInfo[clientIndex];
				// 	evenBuffer = ft_trimString(evenBuffer);
				// 	std::cout << "topic comd: " << buffer << std::endl;
				// 	// check command topic
				// 	channelMsg cmd = checkTopicComd(buffer, currentClient);
					
				// 	// server.clientInfo[clientIndex]._atChannel->setTopic(buffer);
				// 	std::string topicMsg = currentClient._atChannel->channelMessage(cmd, currentClient);
				// 	std::cout << "topicmsg: " << topicMsg << std::endl;
				// 	if (send(server.clientInfo[clientIndex].clientfd, topicMsg.c_str(), topicMsg.size(), 0) < 0)
				// 	{
				// 		std::cout << "setTopic: failed to send\r\n";
				// 		close(server.clientInfo[clientIndex].clientfd);
				// 		continue;
				// 	}          
				// }
			}
		}
	}
}

