#include "Server.hpp"
#include "utils.hpp"
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

//missing the error checks
//also what if port is not set, or password is not set?
void Server::setupServerDetails(Server &server, int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Argument count wrong" << std::endl;
		exit (1);
	}
	//also need to validate password and port
	port = std::stoi(argv[1]);
	pass = argv[2];
	std::cout << "Server's port is: " << port << " and password is : " << pass << std::endl;
}

//one function for setting up the socket?
//missing error checks
void Server::setupSocket()
{
	details.sin_family = AF_INET;
	details.sin_port = htons(port);
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
	newClient.setClientFd(accept4(serverfd, (struct sockaddr *) NULL, NULL, O_NONBLOCK));
	std::cout << "New connection, fd: " << newClient.getClientFd() << std::endl; //debug msg
	clientInfo.push_back(newClient);
	fcntl(newClient.getClientFd(), F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient.getClientFd();
	epoll_ctl(epollfd, EPOLL_CTL_ADD, newClient.getClientFd(), &ev);
}
void Server::handleClient()
{

}
/*Server should have centralized command handler function but this should then call to the specific command functions.
I left this like it is because many of the command functions include parsing which we should preferably handle before we
trigger the actual command function.*/
void Server::handleCommand(Server &server, Client &client, std::string &line)
{
	std::cout << "This is the command: " << line << std::endl;
	if (line.find("CAP") != std::string::npos)
	{
		std::string reply = ":" + server.name + " CAP * LS :multi-prefix\r\n";
		send(client.getClientFd(), reply.c_str(), reply.size(), 0);
		return ;
	}
	if (line.find("PASS") != std::string::npos)
	{
		std::cout << "PASS FOR fd: " << client.getClientFd() << std::endl;
		int start = line.find("PASS ") + 5;
		int end = line.find("\r\n", start);
		std::string password;
		password = line.substr(start, end - start);
		if (password.compare(server.pass) == 0)
		{
			std::cout << "Password matched!" << std::endl;
			client.auth_step++;
		}
		else
			std::cout << "DISASTER" << std::endl;
	}
	if (line.find("NICK") != std::string::npos)
	{
		std::cout << "NICK FOR fd: " << client.getClientFd() << std::endl;
		int start = line.find("NICK ") + 5;
		int end = line.find("\r\n", start);
		std::string nickname;
		nickname = line.substr(start, end - start);
		client.setNick(nickname);
		std::cout << "Nick set: " << client.getNick() << std::endl;
		client.auth_step++;
	}
	if (line.find("USER") != std::string::npos)
	{
		std::cout << "USER FOR fd: " << client.getClientFd() << std::endl;
		std::istringstream iss(line.substr(line.find("USER")));
		std::string command, username, hostname, servername, realname; 
		iss >> command >> username >> hostname >> servername;

		client.setUserName(username);
		client.setHostName(hostname);
		//shouldnt we set the server name for the actual server object?
		client.setServerName(servername);
		std::cout << "User set: " << client.getUserName() << std::endl;
		std::cout << "Host set: " << client.getHostName() << std::endl;
		std::cout << "Server set: " << client.getServerName() << std::endl;
/*		int start = line.find("USER ") + 5;
		int end = line.find(" ", start);
		std::string username;
		username = line.substr(start, end - start);
		client.setUserName(username);
		std::cout << "User set: " << client.getUserName() << std::endl;*/
		if (client.auth_step == 2)
		{
			std::string message = RPL_WELCOME(server.name, client.getNick());
			send(client.getClientFd(), message.c_str(), message.size(), 0);
			std::cout << "We got all the info!" << std::endl;
		}
	}
	if (line.find("PING") != std::string::npos)
	{
		std::cout << "PINGING fd: " << client.getClientFd() << std::endl;
		if (send(client.getClientFd(), "PONG :ft_irc\r\n", sizeof("PONG :ft_irc\r\n") - 1, 0) == -1)
			std::cout << "Send failed" << std::endl;
	}
	if (line.find("JOIN") != std::string::npos)
	{
		line = ft_trimString(line); //trim whitespace
//		client.updateClientInfo(line); //testing with my username
		client.askToJoin(line, server);
	}
}

int Server::getEpollfd() const
{
	return epollfd;
}

struct epoll_event* Server::getEpollEvents()
{
	return events;
}

int Server::getServerfd() const
{
	return serverfd;
}

std::vector<Client>& Server::getClientInfo()
{
	return clientInfo;
}

std::vector<Channel>& Server::getChannelInfo()
{
	return channelInfo;
}
