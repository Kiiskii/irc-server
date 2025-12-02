#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

void Server::parseMessage(Client &c, const std::string &line)
{
	std::vector<std::string> msg;
	std::string command;

	size_t i = 0;
	const size_t n = line.size();
	if (n > MSG_SIZE)
	{
		std::string message = ERR_INPUTTOOLONG(getServerName(), getTarget(c));
		send(c.getClientFd(), message.c_str(), message.size(), 0);
		return ;
	}

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
	
	// std::cout << "" << std::endl;
	// std::cout << "Command: " << command << ", ";
	// for (auto it:msg)
	// 	std::cout << it << " / ";
	// std::cout << std::endl;
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
- When exactly do we return and when should we disconnect?
- Right now you can give PASS, NICK and USER in any order but we may want to change it to PASS first, then NICK/USER in any order
*/
//void Server::handleCommand(Server &server, Client &client, std::string &line)
void Server::handleCommand(Server &server, Client &client, std::string command, std::vector<std::string> &tokens)
{
	//if (command == "CAP")
    //{
    //    std::string reply = ":" + server._name + " CAP * LS :multi-prefix\r\n";
    //    send(client.getClientFd(), reply.c_str(), reply.size(), 0);
    //    return ;
    //}
	if (command == "PASS")
	{
		pass(client, tokens);
	}
	else if (command == "NICK")
	{
		nick(client, tokens);
	}
	else if (command == "USER")
	{
		user(client, tokens);
	}
	else if (client.getClientState() != REGISTERED)
		return;
	else if (command == "PING")
	{
		ping(client, tokens);
	}
	else if (command == "JOIN")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleJoin(client, tokens);
	}
	else if (command == "TOPIC")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleTopic(client, tokens);
	}
	else if (command == "MODE")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleMode(client, tokens);
	}
	else if (command == "INVITE")
	{
		// std::cout << "[" << command << "]" << std::endl;
		// printVector(tokens);
		server.handleInvite(client, tokens);
	}
	else if (command == "PRIVMSG")
	{
		std::cout << "[" << command << "]" << std::endl;
		utils::printVector(tokens);
		server.handlePrivmsg(client, tokens);
	}
	else
	{
		std::string message = ERR_UNKNOWNCOMMAND(getServerName(), getTarget(client), command);
		send(client.getClientFd(), message.c_str(), message.size(), 0);
	}
}
