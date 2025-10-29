#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
 #include <sys/types.h> 
 #include <fcntl.h>
 #include <sys/epoll.h>
//const char ip[]="127.0.0.1"; // for local host
#define MAX_EVENTS 200

class channelInfo
{
	public:
	std::string name = "Empty";
	std::string topic = "Empty";
};


class clientInfo
{
	public:
	int auth_step = 0;
	int clientfd = -1;
	std::string user = "";
	std::string nick = "";
	channelInfo *channel = nullptr;
};

//What should be kept inside class etc...
class serverInfo
{
	public:
	int serverfd = -1;
	std::string pass = "mouse";
	std::string name = "ft_irc";
	std::vector<clientInfo> clientInfo;
	std::vector<channelInfo> channelInfo;
	const int port = 6667;
	struct sockaddr_in details;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];
};

int main(void)
{
	serverInfo server;
	server.details.sin_family = AF_INET;
	server.details.sin_port = htons(6667);
	server.details.sin_addr.s_addr = INADDR_ANY;
	server.serverfd = socket(AF_INET, SOCK_STREAM, 0);
	/*SO_REUSEADDR, allows a socket to bind to an address/port that is still in use. It also
	allows multiple sockets to bind to the same port. So opt here is basically a toggle of whether
	the socket reusing option is enabled or disabled*/
	int opt = 1;
	setsockopt(server.serverfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	bind (server.serverfd, (struct sockaddr *)&server.details, sizeof(server.details));
	if (listen(server.serverfd, 1) == 0)
		std::cout << "We are listening" << std::endl;
	int epollFd;
	epollFd = epoll_create1(0);
	server.event.events = EPOLLIN;
	server.event.data.fd = server.serverfd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, server.serverfd, &server.event);
	while (true)
	{
		int events = epoll_wait(epollFd, server.events, MAX_EVENTS, -1);
		for (int i = 0; i < events; ++i)
		{
			if (server.events[i].data.fd == server.serverfd)
			{
				//New client connection
				clientInfo newClient;
				newClient.clientfd = accept4(server.serverfd, (struct sockaddr *) NULL, NULL, O_NONBLOCK);
				std::cout << "New connection, fd: " << newClient.clientfd << std::endl; //debug msg
				server.clientInfo.push_back(newClient);
				fcntl(newClient.clientfd, F_SETFL, O_NONBLOCK);
				std::string message = ":" + server.name + " 001 kokeilu :Welcome to the " + server.name + " Network, kokeilu\r\n";
				const char *point;
				point = message.c_str();
//				send(newClient.clientfd, point, message.size(), 0);
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = newClient.clientfd;
				epoll_ctl(epollFd, EPOLL_CTL_ADD, newClient.clientfd, &ev);
			}
			else
			{
				char buffer[1024] = {0};
				int clientFd = server.events[i].data.fd;
				int clientIndex;
				for (int i = 0; i < server.clientInfo.size(); i++)
				{
					if (server.clientInfo[i].clientfd == clientFd)
					{
						clientIndex = i;
						break;
					}
				}
				if (recv(server.clientInfo[clientIndex].clientfd, buffer, sizeof(buffer), 0) <= 0)
					std::cout << "Did we encounter a problem" << std::endl;
				std::cout << "Message that we received : [" << buffer << "]" << std::endl;
				std::string evenBuffer(buffer);
				//need to also deal with a situation if password is "empty string"
				if (evenBuffer.find("PASS ") != std::string::npos)
				{
					std::cout << "PASS FOR fd: " << server.clientInfo[clientIndex].clientfd << std::endl;
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
					std::cout << "NICK FOR fd: " << server.clientInfo[clientIndex].clientfd << std::endl;
					int start = evenBuffer.find("NICK ") + 5;
					int end = evenBuffer.find("\r\n", start);
					std::string nickname;
					nickname = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].nick = nickname;
					std::cout << "Nick set: " << server.clientInfo[clientIndex].nick << std::endl;
					server.clientInfo[clientIndex].auth_step++;
				}
				if (evenBuffer.find("USER") != std::string::npos)
				{
					std::cout << "USER FOR fd: " << server.clientInfo[clientIndex].clientfd << std::endl;
					int start = evenBuffer.find("USER ") + 5;
					int end = evenBuffer.find(" ", start);
					std::string username;
					username = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].user = username;
					std::cout << "User set: " << server.clientInfo[clientIndex].user << std::endl;
					if (server.clientInfo[clientIndex].auth_step == 2)
					{
						std::string message = ":" + server.name + " 001 kokeilu :Welcome to the " + server.name + " Network, kokeilu\r\n";
						const char *point;
						point = message.c_str();
						send(server.clientInfo[clientIndex].clientfd, point, message.size(), 0);
						std::cout << "We got all the info!" << std::endl;
					}
				}
				if (evenBuffer.find("PING") != std::string::npos)
				{
					std::cout << "PINGING fd: " << server.clientInfo[clientIndex].clientfd << std::endl;
					if (send(server.clientInfo[clientIndex].clientfd, "PONG :ft_irc\r\n", sizeof("PONG :ft_irc\r\n") - 1, 0) == -1)
						std::cout << "Send failed" << std::endl;
				}
				if (evenBuffer.find("JOIN") != std::string::npos)
				{
					//we need to check if this channel alrdy exists
					std::cout << "JOINING fd: " << server.clientInfo[clientIndex].clientfd << std::endl;
					channelInfo newChannel;
					server.channelInfo.push_back(newChannel);
					server.clientInfo[clientIndex].channel = &newChannel;
					int start = evenBuffer.find("JOIN ") + 5;
					int end = evenBuffer.find("\r\n", start);
					std::string channelname;
					channelname = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].channel->name = channelname;
					std::cout << server.clientInfo[clientIndex].channel->name << std::endl;
					//do we send something here?
				}
				if (evenBuffer.find("TOPIC ") != std::string::npos)
				{
					std::cout << "Buffer contents: [" << evenBuffer << "]" << std::endl;
					int start = evenBuffer.find(":") + 1;
					int end = evenBuffer.find("\r\n", start);
					std::string newtopic;
					newtopic = evenBuffer.substr(start, end - start);
					server.clientInfo[clientIndex].channel->topic = newtopic;
					std::cout << newtopic << std::endl;
					std::string message = ":" + server.name + " 332 " + server.clientInfo[clientIndex].nick + " " + server.clientInfo[clientIndex].channel->name + " " + server.clientInfo[clientIndex].channel->topic + "\r\n";
					const char *point;
					point = message.c_str();
					if (send(server.clientInfo[clientIndex].clientfd, point, message.size(), 0) == -1)
						std::cout << "Send failed" << std::endl;
				}
			}
		}
	}
}
