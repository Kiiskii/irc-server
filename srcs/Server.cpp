#include "Server.hpp"
#include "utils.hpp"

/* @note rememeber to check all on-heap allocated memory, such as chan */
Server::~Server()
{
	for (auto chan : _channelInfo)
		delete chan;
}

/* @def check if the channel exists
	@return ptr to channel if exist else return after the end of vector */
Channel* Server::findChannel(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if ((*it)->getChannelName() == newChannel)
			return *it;
	}
	return nullptr;
}

void Server::printChannelList() const
{
	for (auto i : _channelInfo)
	{
		std::cout << i << std::endl;
	}
}

//missing the error checks
void Server::setupServerDetails(Server &server, int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Argument count wrong" << std::endl;
		exit (1);
	}
	//also need to validate password and port (also set rules for the password, max length??)
	//need to remove ./ from the server name, could just have ircserv as the default name
	_name = argv[0];
	_name.erase(0, _name.find_last_of("/") + 1);
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
/*Handling a new client
- struct sockaddr_in clientAddrress holds the client's IP address and port.
- Calling accept fills the struct with the info (previously this was marked as NULL but then we wouldn't have stored IP anywhere)
- More importantly, accept4 accepts a new connection and returns a new socket fd.
- Now the struct contains the IPv4 address and inet_ntoa converts it into a string. (previously it was in binary)
- Then we make the client socket non blocking, which means the program wont pause waiting for data from this socket.
(means recv() wont pause the program waiting for data. If no data available, it will just return with EAGAIN)
- Epoll is like event manager, it contains a list of sockets that we want to "track", if they communicate*/
void Server::handleNewClient()
{
	Client *newClient = new Client(*this);
	struct sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	newClient->setClientFd(accept4(_serverFd, (struct sockaddr *)&clientAddress, &addressLength, O_NONBLOCK));
	char *clientIP = inet_ntoa(clientAddress.sin_addr);
	newClient->setHostName(clientIP);
	std::cout << "New connection, fd: " << newClient->getClientFd() << std::endl; //debug msg
	_clientInfo.push_back(newClient);
	fcntl(newClient->getClientFd(), F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient->getClientFd();
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClient->getClientFd(), &ev);
}
void Server::handleClient()
{

}

//review this..
void Server::attemptRegister(Client &client)
{
	if (client.getClientState() != REGISTERING)
		return;
	if (client.getNick().empty() || client.getUserName().empty())
		return;
	client.setClientState(REGISTERED);
	std::string message = RPL_WELCOME(_name, client.getNick());
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	message = RPL_YOURHOST(_name, client.getNick(), "1.1");
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	message = RPL_CREATED(_name, client.getNick(), "today");
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	message = RPL_MYINFO(_name, client.getNick(), "1.1", "o", "itkol"); //first refers to user mode, the second channel mode
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	std::string str, str1, str2, str3, str4;
	str = "LINELEN=" + std::to_string(MSG_SIZE);
	str1 = "USERLEN=" + std::to_string(USERLEN);
	str2 = "NICKLEN=" + std::to_string(NICKLEN);
	str3 = "CHANLIMIT=" + std::to_string(MAX_CHANNELS_PER_CLIENT);
	str4 = "CHANMODES=" + std::string(CHANMODES);
	message = RPL_ISUPPORT(_name, client.getNick(), str, str1, str2, str3, str4); // is this the best way to do this...
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	//check if we need something else, message of the day??
	std::cout << "User set: " << client.getUserName() << std::endl;
	std::cout << "Real name set: " << client.getRealName() << std::endl;
	std::cout << "Host set: " << client.getHostName() << std::endl;
	std::cout << "Nick set: " << client.getNick() << std::endl;
	std::cout << "Server set: " << getServerName() << std::endl;
	std::cout << "We got all the info!" << std::endl;
}

/*
- When exactly do we return and when should we disconnect?
- Right now you can give PASS, NICK and USER in any order but we may want to change it to PASS first, then NICK/USER in any order
*/
//void Server::handleCommand(Server &server, Client &client, std::string &line)
void Server::handleCommand(Server &server, Client &client, std::string command, std::vector<std::string> &tokens)
{
	/*Attempting to use stringstream to iterate over the string and then use a vector that contains the tokens*/
	/*
	std::istringstream stream(line);
	std::string command;
	stream >> command; //handling the command separately
	std::vector<std::string> tokens; //this will contain all the command arguments
	std::string token;
	while (stream >> token)
	{
		tokens.push_back(token);
	}
	*/
	//std::string command = tokens[0];
	//std::cout << "Fd is: " << client.getClientFd() << " and cmd and args: " << line << std::endl;
	if (command == "CAP")
    {
        std::string reply = ":" + server._name + " CAP * LS :multi-prefix\r\n";
        send(client.getClientFd(), reply.c_str(), reply.size(), 0);
        return ;
    }
	if (command == "PASS")
	{
		pass(client, tokens);
	}
	if (command == "NICK")
	{
		nick(client, tokens);
	}
	if (command == "USER")
	{
		user(client, tokens);
	}
	if (client.getClientState() != REGISTERED)
		return;
	if (command == "PING")
	{
		ping(client, tokens);
	}
	if (command == "JOIN")
	{
		//std::cout << "\njoin comd: [" << line << "]" << std::endl;

		printVector(tokens);
		server.handleJoin(client, tokens);
	}
	if (command == "TOPIC")
	{
		//std::cout << "\ntopic comd: [" << line << "]" << std::endl;
		printVector(tokens);
		server.handleTopic(client, tokens);
	}
	if (command == "MODE")
	{
		//std::cout << "\nmode comd: [" << line << "]" << std::endl;
		printVector(tokens);
		server.handleMode(client, tokens);
	}
//invalid command?
}

int Server::getEpollfd() const
{
	return _epollFd;
}

std::string& Server::getServerName()
{
	return _name;
}

struct epoll_event* Server::getEpollEvents()
{
	return _events;
}

int Server::getServerfd() const
{
	return _serverFd;
}

std::vector<Client*>& Server::getClientInfo()
{
	return _clientInfo;
}

std::vector<Channel*>& Server::getChannelInfo()
{
	return _channelInfo;
}

/** @return remove the # from the token and return pointer to existing channel, 
 * otherwhile nullpointer */
Channel* Server::setActiveChannel(std::string buffer)
{
	std::string	channelName;

	size_t hashPos = buffer.find("#");
	if (hashPos == std::string::npos)
		return nullptr;
	
	size_t chanEndPos = buffer.find(' ', hashPos);
	if (chanEndPos == std::string::npos)
		chanEndPos = buffer.length();

	channelName = buffer.substr(hashPos + 1, chanEndPos - hashPos -1);
	std::cout << "channelName: [" << channelName << "]" << std::endl;

	return this->findChannel(channelName);
	// for (auto chan : this->_channelInfo)
	// {
	// 	if (chan && chan->getChannelName() == channelName)
	// 		return chan;
	// 	else
	// 	{
	// 		std::cout << "this channel does not exist in server" << std::endl;
	// 		this->_channelInfo
	// 		std::string server = this->_myServer.getServerName(),
	// 			nick = this->getNick();
	
	// 		std::string msg = makeNumericReply(server, ERR_NOTONCHANNEL, nick, {"#" + channelName}, "You're not on that channel");
	// 		if (send(this->getClientFd(), msg.c_str(), msg.size(), 0) < 0)
	// 		{
	// 			std::cout << "joinmsg: failed to send\n";
	// 			return nullptr;
	// 		}
	// 	}
	// }
	// return nullptr;
}




