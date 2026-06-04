#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include <sstream>
#include <csignal>

volatile	sig_atomic_t g_stop = 0;

static void	signalHandler(int sig){
	(void)sig;
	g_stop = 1;
}

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverFd(-1), _maxNickLength(10) {
}

Server::~Server() {
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		close(it->second->getFd());
		delete it->second;
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it){
		delete it->second;
	}
	if (this->_serverFd != -1){
		close(this->_serverFd);
	}
}

void	Server::start() {
	std::signal(SIGINT, signalHandler);

	this->_initSocket();

	std::cout << "Server listening on port " << this->_port << std::endl;

	while (!g_stop) {
		int	ready = poll(&this->_fds[0], this->_fds.size(), -1);
		if (ready < 0) {
			if (errno == EINTR){
				break;
			}
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = 0; i < this->_fds.size(); i++){
			if (!(this->_fds[i].revents & POLLIN)) {
				continue;
			}
			if (this->_fds[i].fd == this->_serverFd) {
				this->_acceptClient();
			}
			else {
				this->_readClient(_clients[this->_fds[i].fd]);
			}
		}
		this->_cleanupClients();
		this->_cleanupChannels();
	}
}

// Creates, configures and binds the server socket. Calls _pushPollFd().
void	Server::_initSocket() {
	this->_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverFd < 0) {
		throw std::runtime_error("socket() failed");
	}

	int	opt = 1;
	if (setsockopt(this->_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::runtime_error("setsockopt() failed");
	}

	if (fcntl(this->_serverFd, F_SETFL, O_NONBLOCK) < 0) {
		throw std::runtime_error("fcntl() failed");
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(this->_port);
	addr.sin_addr.s_addr	= INADDR_ANY;

	if (bind(this->_serverFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		throw std::runtime_error("bind() failed");
	}

	if (listen(this->_serverFd, SOMAXCONN) < 0) {
		throw std::runtime_error("listen() failed");
	}
	this->_pushPollFd(this->_serverFd);
}

// Accepts a new connection. Creates a Client and registers it in _fds and _clients.
void	Server::_acceptClient() {
	struct sockaddr_in	clientAddr;
	socklen_t			clientLen = sizeof(clientAddr);

	int clientFd = accept(this->_serverFd, (struct sockaddr *)&clientAddr, &clientLen);
	if (clientFd < 0) {
		return ;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
		close(clientFd);
		return ;
	}
	this->_pushPollFd(clientFd);
	this->_clients[clientFd] = new Client(clientFd);

	DEBUG_MSG("New client connected (fd=" << clientFd << ")");
}

// Adds a fd to the _fds vector watched by poll().
void	Server::_pushPollFd(int useFd) {
	pollfd	pollFdStruct;

	pollFdStruct.fd			= useFd;
	pollFdStruct.events		= POLLIN;
	pollFdStruct.revents	= 0;
	this->_fds.push_back(pollFdStruct);
}

// Reads data from a client fd. Feeds the buffer and calls _handleLine() per complete line
void	Server::_readClient(Client *client) {
	char	buf[512];
	int fd = client->getFd();
	
	ssize_t	bytes = recv(fd, buf, sizeof(buf) - 1, 0);
	if (bytes <= 0) {
		if (errno == EAGAIN) {
			return;
		}
		if (bytes == 0){
			DEBUG_MSG("Client (fd=" << fd << ") disconnected");
		}
		client->setForDisconnect();
		return ;
	}
	buf[bytes] = '\0';
	this->_clients[fd]->appendBuffer(buf);
	
	while (this->_clients[fd]->hasLine()) {
		std::string	line = this->_clients[fd]->getLine();
		this->_handleLine(client, line);

		if(client->isToDisconnect())
			break;
	}
}

void Server::_cleanupChannels() {
	for (std::map<std::string, Channel*>::iterator chIt = _channels.begin(); chIt != _channels.end(); ) {
		Channel *ch = chIt->second;
		if (ch->getMemberList().empty()) {
			delete ch;
			this->_channels.erase(chIt++);
		}
		else {
			++chIt;
		}
	 }
}

void Server::_cleanupClients(){
	for (std::map<int, Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); ) {
		Client *client = it->second;

		if (!client->isToDisconnect()) {
			++it;
			continue;
		}

		int fd = client->getFd();

		for (std::map<std::string, Channel *>::iterator cit = this->_channels.begin(); cit != this->_channels.end(); ++cit) {
			Channel *ch = cit->second;

			ch->removeMember(client);
			ch->removeInvite(client);
		}

		_nickMap.erase(client->getNick());

		for (size_t i = 0; i < this->_fds.size(); ++i)
		{
			if (this->_fds[i].fd == fd)
			{
				this->_fds.erase(this->_fds.begin() + i);
				break;
			}
		}

		close(fd);
		delete client;
		this->_clients.erase(it++);
	}
}

Client* Server::_getClientByNick(const std::string &nick) {
	std::map<std::string, Client*>::iterator it = this->_nickMap.find(nick);
	if (it == this->_nickMap.end()) {
		return NULL;
	}
	return it->second;
}

Channel* Server::_getChannelByName(const std::string &name) {
	std::map<std::string, Channel*>::iterator it = this->_channels.find(name);
	if (it == this->_channels.end()) {
		return NULL;
	}
	return it->second;
}

bool Server::_isValidNick(const std::string &nick) {
	size_t i = 0;

	if (nick.empty()) {
		return (false);
	}

	if (!std::isalpha(nick[0]) && std::string("[]\\`_^{|}").find(nick[0]) == std::string::npos) {
		return (false);
	}

	i = 1;

	while (i < nick.length()) {
		if (!std::isalnum(nick[i]) && std::string("[]\\`_^{|}-").find(nick[i]) == std::string::npos) {
			return (false);
		}
		++i;
	}

	return (true);
}

bool Server::_reject(Client *client, const std::string &reply) {
	client->sendRaw(reply);
	return false;
}

std::string	Server::debugString() const {
	std::stringstream					ss;
	std::vector<pollfd>::const_iterator	fdIt;
	std::map<int, Client*>::const_iterator	clientIt;
	std::map<std::string, Channel*>::const_iterator	channelIt;

	ss << "=== SERVER ===" << std::endl;

	ss << "Port: " << _port << std::endl;
	ss << "Server FD: " << _serverFd << std::endl;

	ss << "PollFDs (" << _fds.size() << "): ";
	for (fdIt = _fds.begin(); fdIt != _fds.end(); ++fdIt) {
		ss << fdIt->fd;

		if (fdIt + 1 != _fds.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	ss << "Clients (" << _clients.size() << "): ";
	for (clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt) {
		ss << clientIt->second->getPrefix();

		std::map<int, Client*>::const_iterator next = clientIt;
		++next;

		if (next != _clients.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	ss << "Channels (" << _channels.size() << "): ";
	for (channelIt = _channels.begin(); channelIt != _channels.end(); ++channelIt) {
		ss << channelIt->second->getName();

		std::map<std::string, Channel*>::const_iterator next = channelIt;
		++next;

		if (next != _channels.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	return (ss.str());
}
