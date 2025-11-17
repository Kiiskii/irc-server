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
std::vector<Channel*>::iterator Server::isChannelExisting(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if ((*it)->getChannelName() == newChannel)
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
	Client newClient;
	struct sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	newClient.setClientFd(accept4(_serverFd, (struct sockaddr *)&clientAddress, &addressLength, O_NONBLOCK));
	char *clientIP = inet_ntoa(clientAddress.sin_addr);
	newClient.setHostName(clientIP);
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

void Server::attemptRegister(Client &client)
{
//wonder if we can give the alrdy registered feedback here
	if (client.getClientState() != REGISTERING)
		return;
	if (client.getNick().empty() || client.getUserName().empty())
		return;
	client.setClientState(REGISTERED);
	std::string message = RPL_WELCOME(_name, client.getNick());
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	std::cout << "User set: " << client.getUserName() << std::endl;
	std::cout << "Real name set: " << client.getRealName() << std::endl;
	std::cout << "Host set: " << client.getHostName() << std::endl;
	std::cout << "Server set: " << getServerName() << std::endl;
	std::cout << "We got all the info!" << std::endl;
}

/*
- When exactly do we return and when should we disconnect?
- Right now you can give PASS, NICK and USER in any order but we may want to change it to PASS first, then NICK/USER in any order
- Also right now I'm not handling any disconnections at all
*/
void Server::handleCommand(Server &server, Client &client, std::string &line)
{
	/*Attempting to use stringstream to iterate over the string and then use a vector that contains the tokens*/
	std::istringstream stream(line);
	std::string command;
	stream >> command; //handling the command separately
	std::vector<std::string> tokens; //this will contain all the command arguments
	std::string token;
	while (stream >> token)
	{
		tokens.push_back(token);
	}
	std::cout << "Fd is: " << client.getClientFd() << " and cmd and args: " << line << std::endl;
	//still look into this...
	if (command == "CAP")
	{
	 	std::string reply = ":" + server._name + " CAP * LS :multi-prefix\r\n";
	 	send(client.getClientFd(), reply.c_str(), reply.size(), 0);
	 	return ;
	}
	if (command == "PASS")
	{
		pass(server, client, tokens);
	}
	if (command == "NICK")
	{
		nick(server, client, tokens);
	}
	if (command == "USER")
	{
		user(server, client, tokens);
	}
	if (command == "PING")
	{
		ping(server, client, tokens);
	}
	if (command == "JOIN")
	{
		std::cout << "join comd: [" << line << "]" << std::endl;
		client.askToJoin(line, server);
	}
	// if (line.find("JOIN") != std::string::npos)
	// {
	// 	line = ft_trimString(line); //trim whitespace
	// 	std::cout << "join comd: [" << line << "]" << std::endl;
	// 	client.askToJoin(line, server);
	// }
	if (line.find("TOPIC") != std::string::npos)
	{
		line = ft_trimString(line);
		std::cout << "topic comd: [" << line << "]" << std::endl;
		// check command topic
		client.askTopic(line);       
	}
	if (line.find("MODE") != std::string::npos)
	{
		line = ft_trimString(line);
		std::cout << "mode comd: [" << line << "]" << std::endl;
		client.changeMode(line);
	}
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

std::vector<Client>& Server::getClientInfo()
{
	return _clientInfo;
}

std::vector<Channel*>& Server::getChannelInfo()
{
	return _channelInfo;
}

void Server::pass(Server &server, Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(client.getServerName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	if (tokens.size() == 0)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), "PASS");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (tokens[0].compare(server._pass) == 0)
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
	else
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName());
		send(client.getClientFd(), message.c_str(), message.size(), 0);					
	}	
}

/*Nickname rules, characters and length*/
void Server::nick(Server &server, Client &client, std::vector<std::string> tokens)
{
	//so first one cannot have digits but the second one can...
	std::regex pattern(R"(^[A-Za-z\[\]{}\\|][A-Za-z0-9\[\]{}\\|]*$)");
	std::string oldnick = client.getNick();
	if (tokens.size() == 0)
	{
		std::string message = ERR_NONICKNAMEGIVEN(getServerName());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (std::regex_match(tokens[0], pattern) == false)
	{
		std::string message = ERR_ERRONEUSNICKNAME(getServerName(), tokens[0]);
		send(client.getClientFd(), message.c_str(), message.size(), 0);	
		return ;
	}
	for (size_t i = 0; i < server.getClientInfo().size(); i++)
	{
		if (server.getClientInfo()[i].getNick() == tokens[0])
		{
			std::string message = ERR_NICKNAMEINUSE(getServerName(), tokens[0]);
			send(client.getClientFd(), message.c_str(), message.size(), 0);
			return ;
		}
	}
	client.setNick(tokens[0]);
	attemptRegister(client);
	if (client.getClientState() == REGISTERED) // if new nick given, we need to broadcast a message
	{
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);			
	}
}

/*
- Can we remove servername from Client, maybe have a pointer to server if name is needed? So then setservername could be removed from this function
- User name and real name might also have some naming rules and lengths
- Need to check if either user name or real name is empty
- Check the :, and whether we are capturing the entire real name because that can be separated by space

- Do we need to show in which format this needs to be???
*/
void Server::user(Server &server, Client &client, std::vector<std::string> tokens)
{
	if (tokens.size() < 4)
	{
		std::string message = ERR_NEEDMOREPARAMS(getServerName(), "USER");
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;			
	}
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), client.getNick());
		send(client.getClientFd(), message.c_str(), message.size(), 0);
		return ;	
	}
	client.setUserName(tokens[0]);
	std::string realname = "";
	//cleaner way to do this..?
	if (tokens[3].find(":") != std::string::npos)
	{
		tokens[3].erase(0, tokens[3].find(":") + 1);		
	}
	for (int i = 3; i < tokens.size(); i++)
	{
		realname = realname + tokens[i];
		if (i != tokens.size() - 1)
		{
			realname = realname + " ";
		}
	}
	client.setRealName(tokens[3]);
	client.setServerName(getServerName());
	attemptRegister(client);
}

void Server::ping(Server &server, Client &client, std::vector<std::string> tokens)
{
		std::string message = RPL_PONG(tokens[0]);
		send(client.getClientFd(), message.c_str(), message.size(), 0);
}