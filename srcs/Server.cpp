#include "Server.hpp"

/* @def check if the channel exists
	@return ptr to channel if exist else return after the end of vector */
std::vector<Channel>::iterator Server::isChannelExisting(std::string newChannel) 
{
	for (auto it = channelInfo.begin(); it != channelInfo.end(); ++it)
	{
		if ((*it).getChannelName() == newChannel)
			return it;
	}
	return channelInfo.end();
}

void Server::printChannelList() const
{
	for (auto i : channelInfo)
	{
		std::cout << i << std::endl;
	}
}

//one function for setting up the socket?
//missing error checks
void Server::setupSocket()
{
	details.sin_family = AF_INET;
	details.sin_port = htons(6667);
	details.sin_addr.s_addr = INADDR_ANY;
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	/*SO_REUSEADDR, allows a socket to bind to an address/port that is still in use. It also
	allows multiple sockets to bind to the same port. So opt here is basically a toggle of whether
	the socket reusing option is enabled or disabled*/
	int opt = 1;
	setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	bind (serverfd, (struct sockaddr *)&details, sizeof(details));
	if (listen(serverfd, 1) == 0)
		std::cout << "We are listening" << std::endl;
}

//errors missing
void Server::setupEpoll()
{
	epollfd = epoll_create1(0);
	event.events = EPOLLIN;
	event.data.fd = serverfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event);
}

//Here we removed send because even though the client has connected, it doesnt mean they have registered
//Error messages...
void Server::handleNewClient()
{
	Client newClient;
	newClient.clientfd = accept4(serverfd, (struct sockaddr *) NULL, NULL, O_NONBLOCK);
	std::cout << "New connection, fd: " << newClient.clientfd << std::endl; //debug msg
	clientInfo.push_back(newClient);
	fcntl(newClient.clientfd, F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient.clientfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, newClient.clientfd, &ev);
}
void Server::handleClient()
{

}
//Not sure if server is the best place for it but making a centralized location for the commands
void Server::handleCommand(Client &client, std::string &line)
{

}