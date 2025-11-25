#include "Server.hpp"
#include "utils.hpp"

/* @note rememeber to check all on-heap allocated memory, such as chan */
Server::~Server()
{
	for (auto chan : _channelInfo)
		delete chan;
}

/* @def check if the channel exists
	@return ptr to channel if exist else return nullptr */
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

void Server::disconnectClient(Client &client)
{
	//this should remove the client from the channel
	auto it = iterateClients(*this, client);
	if (it == _clientInfo.end())
		return ;
	Client* ptr = *it;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, ptr->getClientFd(), NULL);
	close(ptr->getClientFd());
	getClientInfo().erase(it);
	delete ptr;
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

void Server::parseMessage(Client &c, const std::string &line)
{
	std::vector<std::string> msg;
	std::string command;

	size_t i = 0;
	const size_t n = line.size();

	// skip leading spaces
	i = line.find_first_not_of(' ', i);

	// possibly deal with empty string?
	// if (i == n) return false;

	// command
	size_t cmdStart = i;

	i = line.find(' ', i);

	// no command
	if (cmdStart == i)
		return ;

	//out.command = line.substr(cmdStart, i - cmdStart);
	//msg.push_back(line.substr(cmdStart, i - cmdStart) + ' ');
	command = line.substr(cmdStart, i - cmdStart);
	//msg.push_back(line.substr(cmdStart, i - cmdStart));

	// do we want to normalize to uppercase here?
	for (char &c : command)
		c = std::toupper(static_cast<unsigned char>(c));

	// parameters
	int j = 1;
	while (i < n) {
		// skip spaces before next parameter
		i = line.find_first_not_of(' ', i);
		if (i >= n)
			break ;

		if (line[i] == ':') {
			// handle trailing after ':', trailing should always be last parameter?
			//++i;
			std::string trailing = line.substr(i);
			msg.push_back(trailing);
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			i = line.find(' ', i);
			//msg.push_back(line.substr(paramStart, i - paramStart) + ' ');
			msg.push_back(line.substr(paramStart, i - paramStart));
		}
		++j;
	}
	//if (!msg.empty() && msg.back() == ' ')
	//	msg.pop_back();
	std::cout << "MSG TOKENIZED: " << std::endl;
	std::cout << "Command: " << command << ", ";
	for (auto it:msg)
		std::cout << it << " / ";
	std::cout << std::endl;
	handleCommand(*this, c, command, msg);
}

//these should be under Server class
void Server::receive(Client &c)
{
	// Recieve data from the client
	char buffer[512];
	ssize_t bytes = 1;

	//outMsg.clear();
	
	// DO WE USE MSG_DONTWAIT OR 0???
	while (bytes > 0) {
		bytes = recv(c.getClientFd(), buffer, sizeof(buffer), MSG_DONTWAIT);
		// Errorhandling
		if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
			std::cout << "Failed to recieve from client: " << c.getClientFd() << std::endl;
			disconnectClient(c);
			break ;
		}
		else if (bytes == 0) {
/*Cleaner way to handle this rather than sending in index*/
			std::cout << "Client fd " << c.getClientFd() << " disconnected" << std::endl;
			disconnectClient(c);
			break ;
		}
		// Buffer recieved data
		else {
			c._input.append(buffer, bytes);
			// Check if message if complete
			while (true) {
				size_t newline = c._input.find("\r\n");
				if (newline == c._input.npos)
					break ;
				auto begin = c._input.begin();
				auto end = c._input.begin() + newline;
				parseMessage(c, std::string(begin, end));
				c._input.erase(0, newline + 2);
			}
		}
	}
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


std::string ft_irc_ascii = 
".----------------.  .----------------.  .----------------.  .----------------.  .----------------.  .----------------. \n"
"| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |\n"
"| |  _________   | || |  _________   | || |              | || |     _____    | || |  _______     | || |     ______   | |\n"
"| | |_   ___  |  | || | |  _   _  |  | || |              | || |    |_   _|   | || | |_   __ \\    | || |   .' ___  |  | |\n"
"| |   | |_  \\_|  | || | |_/ | | \\_|  | || |              | || |      | |     | || |   | |__) |   | || |  / .'   \\_|  | |\n"
"| |   |  _|      | || |     | |      | || |              | || |      | |     | || |   |  __ /    | || |  | |         | |\n"
"| |  _| |_       | || |    _| |_     | || |              | || |     _| |_    | || |  _| |  \\ \\_  | || |  \\ `.___.'\\  | |\n"
"| | |_____|      | || |   |_____|    | || |   _______    | || |    |_____|   | || | |____| |___| | || |   `._____.'  | |\n"
"| |              | || |              | || |  |_______|   | || |              | || |              | || |              | |\n"
"| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |\n"
" '----------------'  '----------------'  '----------------'  '----------------'  '----------------'  '----------------'\n";
	send(client.getClientFd(), ft_irc_ascii.c_str(), ft_irc_ascii.size(), 0);
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
	// if (command == "CAP")
    // {
    //     std::string reply = ":" + server._name + " CAP * LS :multi-prefix\r\n";
    //     send(client.getClientFd(), reply.c_str(), reply.size(), 0);
    //     return ;
    // }
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
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleJoin(client, tokens);
	}
	if (command == "TOPIC")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleTopic(client, tokens);
	}
	if (command == "MODE")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleMode(client, tokens);
	}
	if (command == "INVITE")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleInvite(client, tokens);
	}
	if (command == "PRIVMSG")
	{
		std::cout << "[" << command << "]" << std::endl;
		printVector(tokens);
		server.handlePrivmsg(client, tokens);
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
