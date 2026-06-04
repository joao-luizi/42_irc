#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <poll.h>
#include <iostream>
#include <cstdlib>

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "Debug.hpp"
#include "Replies.hpp"
class	Server{
	private:
		int								_port;
		std::string						_password;
		int								_serverFd;
		size_t 							_maxNickLength;
		std::vector<pollfd>				_fds;
		std::map<int, Client*>			_clients;
		std::map<std::string, Channel*>	_channels;
		std::map<std::string, Client*>  _nickMap;

		Server();
		Server(const Server &other);
		Server &operator=(const Server &other);

		//Server.cpp
		void		_initSocket();
		void		_acceptClient();
		void		_pushPollFd(int useFd);
		void		_readClient(Client *client);
		void		_cleanupClients();
		void		_cleanupChannels();
		Client*		_getClientByNick(const std::string &nick);
		Channel*	_getChannelByName(const std::string &name);
		bool		_reject(Client *client, const std::string &reply);
		std::string	debugString() const;

		//ServerAuth.cpp
		void	_broadcastNickChange(Client *client, const std::string &oldNick, const std::string &newNick);
		void	_notifyNickSelf(Client *client, const std::string &oldNick, const std::string &newNick);
		void	_applyNickChange(Client *client, const std::string &nick, std::string &oldNick);
		bool	_isNickValidRequest(const std::string &nick);
		void	_handleLine(Client *client, const std::string &line);
		void	_manageCmd(Client *client, const ParsedCommand &cmd);
		void	_handleNick(Client *client, const ParsedCommand &cmd);
			
		//ServerChannel.cpp
		void	_handleJoin(Client *client, const ParsedCommand &cmd);
		void	_handlePart(Client *client, const ParsedCommand &cmd);

		//ServerMsg.cpp
		void	_handlePrivmsg(Client *client, const std::string &target, const std::string &trailing);

		//ServerOp.cpp
		bool	_validateCommand(Client *client, const ParsedCommand &cmd);
		bool	_requiresRegistration(const ParsedCommand &cmd);
		bool	_isValidNick(const std::string &nick);
		void	_handleQuit(Client *client, const ParsedCommand &cmd);
		void	_handleKick(Client* client, const ParsedCommand &cmd);
		void	_handleMode(Client* client, const ParsedCommand &cmd);
		void	_applyChannelModes(Client *op, Channel *channel, const ParsedCommand &cmd);
		void	_handleShow(Client *client, const ParsedCommand &cmd);
		void	_handleInvite(Client *client, const ParsedCommand &cmd);
		void	_handleTopic(Client *client, const ParsedCommand &cmd);
		void	_handleWho(Client *client, const ParsedCommand &cmd);
		void	_handlePass(Client *client, const ParsedCommand &cmd);
		void	_handleUser(Client *client, const ParsedCommand &cmd);



	public:
		Server(int port, const std::string &password);
		~Server();

		void	start();
};

#endif