#include "Server.hpp"
#include "utils.hpp"
/* @def check if the channel exists
	@return ptr to channel if exist else return after the end of vector */
std::vector<Channel>::iterator Server::isChannelExisting(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if ((*it).getChannelName() == newChannel)
			return it;
	}
	return _channelInfo.end();
}

void Server::printChannelList() const
{
	for (auto i : _channelInfo)
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
	_port = std::stoi(argv[1]);
	_pass = argv[2];
	std::cout << "Server's port is: " << _port << " and password is : " << _pass << std::endl;
}

//one function for setting up the socket?
//missing error checks
void Server::setupSocket()
{
	_details.sin_family = AF_INET;
	_details.sin_port = htons(_port);
	_details.sin_addr.s_addr = INADDR_ANY;
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	/*SO_REUSEADDR, allows a socket to bind to an address/port that is still in use. It also
	allows multiple sockets to bind to the same port. So opt here is basically a toggle of whether
	the socket reusing option is enabled or disabled*/
	int opt = 1;
	setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	bind (_serverFd, (struct sockaddr *)&_details, sizeof(_details));
	if (listen(_serverFd, 1) == 0)
		std::cout << "We are listening" << std::endl;
}

//errors missing
void Server::setupEpoll()
{
	_epollFd = epoll_create1(0);
	_event.events = EPOLLIN;
	_event.data.fd = _serverFd;
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &_event);
}

//Here we removed send because even though the client has connected, it doesnt mean they have registered
//Error messages...
void Server::handleNewClient()
{
	Client newClient;
	newClient.setClientFd(accept4(_serverFd, (struct sockaddr *) NULL, NULL, O_NONBLOCK));
	std::cout << "New connection, fd: " << newClient.getClientFd() << std::endl; //debug msg
	_clientInfo.push_back(newClient);
	fcntl(newClient.getClientFd(), F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient.getClientFd();
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClient.getClientFd(), &ev);
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
	// if (line.find("CAP") != std::string::npos)
	// {
	// 	std::string reply = ":" + server.name + " CAP * LS :multi-prefix\r\n";
	// 	send(client.getClientFd(), reply.c_str(), reply.size(), 0);
	// 	return ;
	// }
	if (line.find("PASS") != std::string::npos)
	{
		std::cout << "PASS FOR fd: " << client.getClientFd() << std::endl;
		int start = line.find("PASS ") + 5;
		int end = line.find("\r\n", start);
		std::string password;
		password = line.substr(start, end - start);
		if (password.compare(server._pass) == 0)
		{
			std::cout << "Password matched!" << std::endl;
			client.auth_step++;
		}
		else
			std::cout << "DISASTER" << std::endl;
	}

	if (line.find("NICK") != std::string::npos)
	{
		//we also need to make sure the registration happens in order
		//missing errors if nickname is empty, if contains wrong character or if no nick given? (we still need to check if the person doesnt even give NICK as the command but also if NICK is empty)
		//but the if no NICK given check needs to be done elsewhere
		std::string oldnick = client.getNick();
		std::cout << "NICK FOR fd: " << client.getClientFd() << std::endl;
		int start = line.find("NICK ") + 5;
		int end = line.find("\r\n", start);
		std::string nickname;
		nickname = line.substr(start, end - start);
		client.setNick(nickname);
		std::cout << "Nick set: " << client.getNick() << std::endl;
		client.auth_step++;
		//go over the flow for this one, we should not be sending the new nick message if its the first time updating valid nick
		if (client.getNick().size() != 0 && line.find("NICK ") != std::string::npos) // if new nick given, we need to broadcast a message
		{
			std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());
			send(client.getClientFd(), message.c_str(), message.size(), 0);			
		}
		if (client.getNick().size() == 0)
		{
			std::string placeholderserv = "localhost";
			std::string message = ERR_NONICKNAMEGIVEN(placeholderserv);
			send(client.getClientFd(), message.c_str(), message.size(), 0);		
		}
	}
	if (line.find("USER") != std::string::npos)
	{
		//nickname in use for someone on the server
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
			std::string message = RPL_WELCOME(server._name, client.getNick());
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
	if (line.find("TOPIC") != std::string::npos)
	{
		line = ft_trimString(line);
		std::cout << "topic comd: [" << line << "]" << std::endl;
		// check command topic
		client.askTopic(line);       
	}
}

int Server::getEpollfd() const
{
	return _epollFd;
}

struct epoll_event* Server::getEpollEvents()
{
	return _events;
}

int Server::getServerfd() const
{
	return _serverFd;
}

std::vector<Client>& Server::getClientInfo()
{
	return _clientInfo;
}

std::vector<Channel>& Server::getChannelInfo()
{
	return _channelInfo;
}
