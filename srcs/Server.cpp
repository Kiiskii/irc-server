#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

/* @note rememeber to check all on-heap allocated memory, such as chan, client */
Server::~Server()
{
	std::cout << "We have received a signal or the server simply failed to start!" << std::endl;
	for (auto chan : _channelInfo)
		delete chan;
	for (auto client : _clientInfo)
		disconnectClient(client);
	if (_epollFd != -1)	
		close(_epollFd);
	if (_serverFd != -1)
		close(_serverFd);
}

/* @def check if the channel exists
	@return ptr to channel if exist else return nullptr */
Channel* Server::findChannel(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if (utils::compareCasemappingStr((*it)->getChannelName(), newChannel))
			return *it;
	}
	return nullptr;
}

void Server::disconnectClient(Client *client)
{
	//this should remove the client from the channel as well
	auto it = std::find(_clientInfo.begin(), _clientInfo.end(), client);
	if (it == _clientInfo.end())
		return ;
	Client* ptr = *it;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, ptr->getClientFd(), NULL); //this fails if fd is already closed, its alrady removed etc so no need to protect this
	close(ptr->getClientFd());
	getClientInfo().erase(it);
	delete ptr;
}

/*Port is a 16-bit unsigned int, meaning valid range is 0-65535.
However, ports under 1024 are privileged and require root privileges. In our program,
we would get an error of failing to bind a server socket.*/
void Server::setupServerDetails(Server &server, int argc, char *argv[])
{
	size_t pos;

	_name = argv[0];
	_name.erase(0, _name.find_last_of("/") + 1);
	try
	{	_port = std::stoi(argv[1], &pos); }
	catch (const std::exception&) //invalid argument ("abc") or out of range
	{ 
		std::cerr << ERR_PORT << std::endl;
		exit (1);
	}
	std::string s = argv[1];
	if (pos != s.length() || _port < 1024 || _port > 65535) //trailing invalid characters and if port out of range
	{
		std::cerr << ERR_PORT << std::endl;
		exit(1);
	}
	_pass = argv[2];
	std::cout << "Server's port is: " << _port << " and password is : " << _pass << std::endl;
}

/*SO_REUSEADDR, allows a socket to bind to an address/port that is still in use. It also
allows multiple sockets to bind to the same port. So opt here is basically a toggle of whether
the socket reusing option is enabled or disabled*/
void Server::setupSocket()
{
	_details.sin_family = AF_INET;
	_details.sin_port = htons(_port);
	_details.sin_addr.s_addr = INADDR_ANY;
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1) //happens when you hit ulimit -n or too many connections (open fds)
	{
		std::cerr << ERR_SOCKET << std::endl;
		exit (1);
	}
	int opt = 1;
	setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	if (bind(_serverFd, (struct sockaddr *)&_details, sizeof(_details)) == -1) //fails when port already in use (try port under 1024?)
	{
		std::cerr << ERR_BIND << std::endl;
		exit (1);
	}
	if (listen(_serverFd, 1) == -1) //not so likely to fail
	{
		std::cerr << ERR_LISTEN << std::endl;
		exit (1);
	}
}

void Server::setupEpoll()
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1) //again, too many epoll fds open, system limits, mem, try ulimit -n
	{
		std::cerr << ERR_EPOLL << std::endl;
		exit (1);
	}
	_event.events = EPOLLIN;
	_event.data.fd = _serverFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &_event) == -1) // fd is invalid, adding fd twice, too many, mem, try to call this with invalid fd
	{
		std::cerr << ERR_EPOLLCTL << std::endl;
		exit (1);
	}
}

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
	if (newClient->getClientFd() == -1) //clients disconnect too quickly, fd exhaustion, race condition, try to connect and instantly close with ctrl C
	{
		std::cerr << ERR_ACCEPT << std::endl;
		exit (1);
	}
	char *clientIP = inet_ntoa(clientAddress.sin_addr);
	newClient->setHostName(clientIP);
	_clientInfo.push_back(newClient);
	fcntl(newClient->getClientFd(), F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient->getClientFd();
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClient->getClientFd(), &ev) == -1) // fd is invalid, adding fd twice, too many, mem, try to call this with invalid fd
	{
		std::cerr << ERR_EPOLLCTL << std::endl;
		exit (1);		
	}
	std::cout << "New connection, fd: " << newClient->getClientFd() << std::endl; //debug msg
}

/*
- Additional info to put here, at least Trang added the channel length!*/
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
	"CHANLIMIT=" + std::to_string(CHANLIMIT),
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
