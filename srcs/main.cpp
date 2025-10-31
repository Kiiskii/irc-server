
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"

int main(void)
{
	Server server;
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
				//need to also deal with a situation if password is "empty string"
				if (evenBuffer.find("PASS ") != std::string::npos)
				{
					std::cout << "PASS FOR fd: " << server.clientInfo[clientIndex].getClientFd() << std::endl;
					int start = evenBuffer.find("PASS ") + 5;
					int end = evenBuffer.find("\r\n", start);
					std::string password;
					password = evenBuffer.substr(start, end - start);
					if (password.compare(server.pass) == 0)
					{
						std::cout << "Password matched!" << std::endl;
						server.clientInfo[clientIndex].auth_step++;
					}
					else
						std::cout << "DISASTER" << std::endl;
				}
				if (evenBuffer.find("NICK") != std::string::npos)
				{
					std::cout << "NICK FOR fd: " << server.clientInfo[clientIndex].getClientFd() << std::endl;
					int start = evenBuffer.find("NICK ") + 5;
					int end = evenBuffer.find("\r\n", start);
					std::string nickname;
					nickname = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].setNick(nickname);
					std::cout << "Nick set: " << server.clientInfo[clientIndex].getNick() << std::endl;
					server.clientInfo[clientIndex].auth_step++;
				}
				if (evenBuffer.find("USER") != std::string::npos)
				{
					std::cout << "USER FOR fd: " << server.clientInfo[clientIndex].getClientFd() << std::endl;
					int start = evenBuffer.find("USER ") + 5;
					int end = evenBuffer.find(" ", start);
					std::string username;
					username = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].setUserName(username);
					std::cout << "User set: " << server.clientInfo[clientIndex].getUserName() << std::endl;
					if (server.clientInfo[clientIndex].auth_step == 2)
					{
						std::string message = ":" + server.name + " 001 kokeilu :Welcome to the " + server.name + " Network, kokeilu\r\n";
						const char *point;
						point = message.c_str();
						send(server.clientInfo[clientIndex].getClientFd(), point, message.size(), 0);
						std::cout << "We got all the info!" << std::endl;
					}
				}
				if (evenBuffer.find("PING") != std::string::npos)
				{
					std::cout << "PINGING fd: " << server.clientInfo[clientIndex].getClientFd() << std::endl;
					if (send(server.clientInfo[clientIndex].getClientFd(), "PONG :ft_irc\r\n", sizeof("PONG :ft_irc\r\n") - 1, 0) == -1)
						std::cout << "Send failed" << std::endl;
				}
				
				if (evenBuffer.find("JOIN") != std::string::npos)
				{
					evenBuffer = ft_trimString(evenBuffer); //trim whitespace
					Client& currentClient = server.clientInfo[clientIndex];
					currentClient.updateClientInfo(evenBuffer); //testing with my username
					currentClient.askToJoin(evenBuffer, server);
				}

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

