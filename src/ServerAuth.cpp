#include "Server.hpp"
#include <iostream>

// Parses a raw line into cmd and arg. Calls _manageCmd(). Sends 001 welcome on first registration.
void Server::_handleLine(Client *client, const std::string &line) {
	
	if (client->isToDisconnect())
		return;

	const ParsedCommand cmd = Parser::parse(line);
	
	DEBUG_MSG(cmd);

	if (!_validateCommand(client, cmd)) {
		return;
	}
	
	if (!client->isRegistered() && _requiresRegistration(cmd)) {
		client->sendRaw(Replies::notRegistered());
		return;
	}
	this->_manageCmd(client, cmd);

	if (client->isRegistered() && !client->isWelcomed()) {
		client->setWelcomed();
		client->sendRaw(Replies::rplWelcome(client->getNick()));
		client->sendRaw(Replies::rplYourHost(client->getNick()));
		client->sendRaw(Replies::rplCreated(client->getNick()));
		client->sendRaw(Replies::rplMyInfo(client->getNick()));
	}
}

// Dispatches PASS, NICK, USER, JOIN, PRIVMSG to their handlers.
void Server::_manageCmd(Client *client, const ParsedCommand &cmd) {
	
	
	switch (cmd.type) {
		case PASS:
			this->_handlePass(client, cmd);
			break;
		case NICK:
			this->_handleNick(client, cmd);
		break;
		case USER:
			this->_handleUser(client, cmd);
		break;
		case JOIN:
			this->_handleJoin(client, cmd);
		break;
		case PART:
			this->_handlePart(client, cmd);
			break;
		case PRIVMSG:
			this->_handlePrivmsg(client, cmd.params[0], cmd.trailing);
		break;
		case QUIT:
			this->_handleQuit(client, cmd);
		break;
		case KICK:
			this->_handleKick(client, cmd);
		break;
		case MODE:
			this->_handleMode(client, cmd);
		break;
		case SHOW:
			this->_handleShow(client, cmd);
		break;
		case INVITE:
			this->_handleInvite(client, cmd);
		break;
		case TOPIC:
			this->_handleTopic(client, cmd);
		break;
		case WHO:
			this->_handleWho(client, cmd);
		break;
		case CAP:
			break;
		default:
			client->sendRaw(Replies::unknownCommand(cmd.command));
			break;
	}
}

void Server::_handlePass(Client *client, const ParsedCommand &cmd) {
	if (client->isRegistered() || client->hasPassOk()) {
		client->sendRaw(Replies::errAlreadyRegistered(client->getNick()));
		return;
	}
	if (cmd.params.size() > 1) {
		client->sendRaw(Replies::errTooManyParams(client->getNick(), "PASS"));
		return;
	}
	if (cmd.params[0] == this->_password) {
		client->setPassOk();
		return;
	}
	client->sendRaw(Replies::errPassMismatch());
	client->setForDisconnect();
}

void Server::_handleUser(Client *client, const ParsedCommand &cmd) {
	if (client->isRegistered() || !client->getUser().empty()) {
		client->sendRaw(Replies::errAlreadyRegistered(client->getNick()));
		return;
	}

	client->setUser(cmd.params[0]);

	if (!cmd.trailing.empty()) {
		client->setRealName(cmd.trailing);
	}
}


bool Server::_isNickValidRequest(const std::string &nick) {
	if (nick.empty()) {
		return false;
	}

	if (nick.length() > _maxNickLength) {
		return false;
	}

	if (!_isValidNick(nick)) {
		return false;
	}

	return true;
}

void Server::_applyNickChange(Client *client, const std::string &nick, std::string &oldNick) {
	oldNick = client->getNick();

	if (!oldNick.empty()) {
		_nickMap.erase(oldNick);
	}

	client->setNick(nick);
	client->setNickOk();

	_nickMap[nick] = client;
}

void Server::_notifyNickSelf(Client *client, const std::string &oldNick, const std::string &newNick) {
	client->sendRaw(Replies::nickChange(oldNick, client->getUser(), newNick));
}

void Server::_broadcastNickChange(Client *client, const std::string &oldNick, const std::string &newNick) {
	std::set<int> recipients;

	const std::set<std::string>& channels = client->getChannels();

	for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		std::map<std::string, Channel*>::iterator chIt = _channels.find(*it);
		if (chIt == _channels.end()) {
			continue;
		}

		Channel *channel = chIt->second;
		const std::map<int, Client *>& members = channel->getMembers();

		for (std::map<int, Client *>::const_iterator mit = members.begin(); mit != members.end(); ++mit) {
			recipients.insert(mit->first);
		}
	}

	for (std::set<int>::const_iterator rit = recipients.begin(); rit != recipients.end(); ++rit) {
		if (*rit != client->getFd()) {
			_clients[*rit]->sendRaw(Replies::nickChange(oldNick, client->getUser(), newNick));
		}
	}
}

void Server::_handleNick(Client *client, const ParsedCommand &cmd) {
	std::string nick = cmd.params[0];

	size_t space = nick.find(' ');
	if (space != std::string::npos) {
		nick = nick.substr(0, space);
	}

	if (client->getNick() == nick) {
		return;
	}

	if (this->_nickMap.find(nick) != this->_nickMap.end()) {
		client->sendRaw(Replies::nickInUse(nick));
		return;
	}

	if (!this->_isNickValidRequest(nick)) {
		client->sendRaw(Replies::erroneousNick(nick));
		return;
	}


	std::string oldNick;
	this->_applyNickChange(client, nick, oldNick);

	// Always notify self (even pre-registration)
	if (!oldNick.empty()){
		this->_notifyNickSelf(client, oldNick, nick);}

	// Only broadcast if fully registered
	if (!client->isRegistered())
		return;

	this->_broadcastNickChange(client, oldNick, nick);
}
