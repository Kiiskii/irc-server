#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

/* @note rememeber to check all on-heap allocated memory, such as chan, client */
Server::~Server()
{
	for (auto chan : _channelInfo)
		delete chan;
	for (auto client : _clientInfo)
		delete client;
}

/* @def check if the channel exists
	@return ptr to channel if exist else return nullptr */
Channel* Server::findChannel(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if (utils::ft_stringToLower((*it)->getChannelName()) == utils::ft_stringToLower(newChannel))
			return *it;
	}
	return nullptr;
}

void Server::disconnectClient(Client &client)
{
	//this should remove the client from the channel as well
	auto it = iterateClients(*this, client);
	if (it == _clientInfo.end())
		return ;
	Client* ptr = *it;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, ptr->getClientFd(), NULL);
	close(ptr->getClientFd());
	getClientInfo().erase(it);
	delete ptr;
}

void Server::setupServerDetails(Server &server, int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << INPUT_FORMAT << std::endl;
		exit (1);
	}
	//could just have ircserv as the default name rather than getting it here...
	_name = argv[0];
	_name.erase(0, _name.find_last_of("/") + 1);
	try
	{	_port = std::stoi(argv[1]); }
	catch (const std::invalid_argument&) //also this catches strings but doesnt catch 6667a for example
	{ 
		std::cerr << ERR_PORT << std::endl;
		exit (1);
	}
	//also should have a check for valid port nbr
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

//What does it mean if epoll_ctl fails?
void Server::setupEpoll()
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
	{
		//failed to open an epoll file descriptor
	}
	_event.events = EPOLLIN;
	_event.data.fd = _serverFd;
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &_event);
}

//I believe accept needs to be protected but investigate the rest because I'm not sure in which scenario they would fail
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
	if (newClient->getClientFd() == -1)
	{
		//failed to accept a connection on a socket...
	}
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

/*
- Missing message of the day
- Additional info to put here?*/
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
	message = RPL_MYINFO(_name, client.getNick(), "1.1", "o", "itkol");
	send(client.getClientFd(), message.c_str(), message.size(), 0);
	std::vector<std::string> info = 
	{
	"LINELEN=" + std::to_string(MSG_SIZE),
	"USERLEN=" + std::to_string(USERLEN),
	"NICKLEN=" + std::to_string(NICKLEN),
	"CHANLIMIT=" + std::to_string(MAX_CHANNELS_PER_CLIENT),
	"CHANMODES=" + std::string(CHANMODES)
	};
	std::string infoPack;
	for (int i = 0; i < info.size(); i++)
		infoPack = infoPack + info[i] + " ";
	message = RPL_ISUPPORT(_name, client.getNick(), infoPack);
	send(client.getClientFd(), message.c_str(), message.size(), 0);
//needs \r\n both
std::string ft_irc_ascii = 
":" + getServerName() + " 375 " + client.getNick() + " :- " + getServerName() + " Message of the day -\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + ".----------------.  .----------------.  .----------------.  .----------------.  .----------------.  .----------------. \n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| |  _________   | || |  _________   | || |              | || |     _____    | || |  _______     | || |     ______   | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| | |_   ___  |  | || | |  _   _  |  | || |              | || |    |_   _|   | || | |_   __ \\    | || |   .' ___  |  | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| |   | |_  \\_|  | || | |_/ | | \\_|  | || |              | || |      | |     | || |   | |__) |   | || |  / .'   \\_|  | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| |   |  _|      | || |     | |      | || |              | || |      | |     | || |   |  __ /    | || |  | |         | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| |  _| |_       | || |    _| |_     | || |              | || |     _| |_    | || |  _| |  \\ \\_  | || |  \\ `.___.'\\  | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| | |_____|      | || |   |_____|    | || |   _______    | || |    |_____|   | || | |____| |___| | || |   `._____.'  | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| |              | || |              | || |  |_______|   | || |              | || |              | || |              | |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + "| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |\n"
":" + getServerName() + " 372 " + client.getNick() + " :" + " '----------------'  '----------------'  '----------------'  '----------------'  '----------------'  '----------------'\n"
":" + getServerName() + " 372 " + client.getNick() + " :Created by Karoliina Hiidenheimo, Trang Pham and Anton Kiiski.\n"
":" + getServerName() + " 376 " + client.getNick() + " :End of /MOTD command.\n";
	send(client.getClientFd(), ft_irc_ascii.c_str(), ft_irc_ascii.size(), 0);
	std::cout << "User set: " << client.getUserName() << std::endl;
	std::cout << "Real name set: " << client.getRealName() << std::endl;
	std::cout << "Host set: " << client.getHostName() << std::endl;
	std::cout << "Nick set: " << client.getNick() << std::endl;
	std::cout << "Server set: " << getServerName() << std::endl;
	std::cout << "We got all the info!" << std::endl;
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

void Server::removeChannel(Channel* chann)
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end();) {
		if ((*it) == chann)
			it = _channelInfo.erase(it);
		else
			++it;
	}
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
	// std::cout << "channelName: [" << channelName << "]" << std::endl;

	return this->findChannel(channelName);
}

Client*	Server::findClient(std::string nickName)
{
	for (auto it = _clientInfo.begin(); it != _clientInfo.end(); ++it)
	{
		if ((*it)->getNick() == nickName)
			return *it;
	}
	return nullptr;
}
