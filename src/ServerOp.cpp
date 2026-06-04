#include "Server.hpp"

bool Server::_validateCommand(Client *client, const ParsedCommand &cmd) {
	// 1. CONTROL CHAR CHECK (global rule)
	if (Parser::hasControlChars(cmd.trailing)) {
		return this->_reject(client, Replies::errInvalidParams(client->getNick(), cmd.command));
	}

	for (size_t i = 0; i < cmd.params.size(); ++i) {
		if (Parser::hasControlChars(cmd.params[i])) {
			return this->_reject(client, Replies::errInvalidParams(client->getNick(), cmd.command));
		}
	}

	// 2. STRUCTURAL VALIDATION
	switch (cmd.type) {
		case PASS:
			if (cmd.params.empty()) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}
			if (cmd.params.size() > 1) {
				return this->_reject(client, Replies::errTooManyParams(client->getNick(), cmd.command));
			}
			break;

		case NICK:
			if (cmd.params.empty()) {
				return this->_reject(client, Replies::noNickGiven());
			}
			if (cmd.params.size() > 1) {
				return this->_reject(client, Replies::errTooManyParams(client->getNick(), cmd.command));
			}
			break;

		case USER:
			if (cmd.params.size() < 3 || cmd.trailing.empty()) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}

			if (cmd.params.size() > 3) {
				return this->_reject(client, Replies::errTooManyParams(client->getNick(), cmd.command));
			}
			break;

		case JOIN:
			if (cmd.params.size() < 1) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}
			break;

		case PART:
			if (cmd.params.empty()) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}
			break;

		case PRIVMSG:
			if (cmd.params.empty()) {
				return this->_reject(client, Replies::errNoRecipient(client->getNick()));  // 411
			}
			break;

		case KICK:
			if (cmd.params.size() < 2) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}
			break;

		case INVITE:
			if (cmd.params.size() < 2) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}

			if (cmd.params.size() > 2) {
				return this->_reject(client, Replies::errTooManyParams(client->getNick(), cmd.command));
			}
			break;

		case TOPIC:
		case MODE:
		case WHO:
		case SHOW:
			if (cmd.params.empty()) {
				return this->_reject(client, Replies::needMoreParams(client->getNick(), cmd.command));
			}
			break;

		case QUIT:
			// QUIT may or may not have a trailing message
			// so we only validate control chars (already done above)
			break;
		
		case CAP:
			break;
		default:
			return this->_reject(client, Replies::unknownCommand(cmd.command));
	}
	return true;
}

bool Server::_requiresRegistration(const ParsedCommand &cmd) {
	switch (cmd.type) {
		case PASS:
		case NICK:
		case USER:
		case QUIT:
		case SHOW:
			return false;
		case CAP:
			return false;
		default:
			return true;
	}
}

void Server::_handleQuit(Client *client, const ParsedCommand &cmd) {
	client->setForDisconnect();

	std::string reason = cmd.trailing.empty() ? "Client Quit" : cmd.trailing;

	std::string msg = Replies::quit(client->getPrefix(), reason);

	std::set<std::string> channels = client->getChannels();

	for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
		std::map<std::string, Channel*>::iterator ch = _channels.find(*it);

		if (ch != _channels.end()) {
			ch->second->broadcast(msg, client->getFd());
		}
	}

	DEBUG_MSG("Client (fd=" << client->getFd() << ") requested QUIT");
}

void Server::_handleKick(Client* client, const ParsedCommand &cmd) {
	std::map<std::string, Channel*>::iterator it = _channels.find(cmd.params[0]);

	if (it == _channels.end()) {
		client->sendRaw(Replies::errNoSuchChannel(client->getNick(), cmd.params[0]));
		return;
	}

	Channel *channel = it->second;

	if (!channel->hasMember(client->getFd())) {
		client->sendRaw(Replies::errNotOnChannel(client->getNick(), channel->getName()));
		return;
	}

	if (!channel->isOperator(client->getFd())) {
		client->sendRaw(Replies::errNotChannelOp(client->getNick(), channel->getName()));
		return;
	}

	std::map<std::string, Client*>::iterator itClient = _nickMap.find(cmd.params[1]);

	if (itClient == _nickMap.end()) {
		client->sendRaw(Replies::errNoSuchNick(client->getNick(), cmd.params[1]));
		return;
	}

	Client *target = itClient->second;

	if (!channel->hasMember(target->getFd())) {
		client->sendRaw(Replies::errUserNotOnChannel( client->getNick(), target->getNick(), channel->getName()));
		return;
	}

	std::string reason = cmd.trailing.empty() ? client->getNick() : cmd.trailing;

	std::string msg = Replies::kick(client->getPrefix(), channel->getName(), target->getNick(), reason);

	channel->broadcast(msg, -1);

	channel->removeMember(target);

	if (channel->getMembers().empty()) {
		delete channel;
		this->_channels.erase(it);
	}

	target->removeChannel(cmd.params[0]);
}

void Server::_handleMode(Client *client, const ParsedCommand &cmd) {
	if (cmd.params.size() < 1) {
		return;
	}

	const std::string &channelName = cmd.params[0];

	if (channelName.empty() || channelName[0] != '#') {
		client->sendRaw(Replies::errInvalidChannelName(client->getNick(), channelName));
		return;
	}

	std::map<std::string, Channel*>::iterator it = this->_channels.find(channelName);

	if (it == _channels.end()) {
		client->sendRaw(Replies::errNoSuchChannel(client->getNick(), channelName));
		return;
	}

	Channel *channel = it->second;

	if (!channel->hasMember(client->getFd())) {
		client->sendRaw(Replies::errNotOnChannel(client->getNick(), channelName));
		return;
	}

	// MODE query: MODE #channel
	if (cmd.params.size() == 1) {
		client->sendRaw(Replies::modeReply(client->getNick(), channelName, channel->getModes()));
		return;
	}

	if (!channel->isOperator(client->getFd())) {
		client->sendRaw(Replies::errNotChannelOp(client->getNick(), channelName));
		return;
	}

	this->_applyChannelModes(client, channel, cmd);
}

void Server::_applyChannelModes(Client *op, Channel *channel, const ParsedCommand &cmd) {
	std::string modes = cmd.params[1];
	size_t argIndex = 2;
	bool adding = true;
	std::string addstr = "";
	for (size_t i = 0; i < modes.size(); i++) {
		char m = modes[i];

		if (m == '+') {
			adding = true;
			addstr = "+";
			continue;
		}
		if (m == '-') {
			adding = false;
			addstr = "-";
			continue;
		}

		switch (m) {
			case 'i':
				channel->setInviteOnly(adding);
				break;

			case 't':
				channel->setTopicRestricted(adding);
				break;

			case 'k':
				if (adding) {
					if (argIndex >= cmd.params.size()) {
						return;
					}
					channel->setKey(cmd.params[argIndex++]);
				}
				else {
					channel->removeKey();
				}
				break;

			case 'o':
				if (argIndex < cmd.params.size()) {
					Client *target = this->_getClientByNick(cmd.params[argIndex++]);
					if (target) {
						if (channel->setOperator(target, adding)) {
							std::string mode = addstr + "o";

							std::string msg = Replies::modeChange(op->getPrefix(), channel->getName(), mode, target->getNick());
							channel->broadcast(msg, -1); // -1 sends to everyone and excluded no fd
						}
					}
				}
				break;
			case 'l':
				if (adding && argIndex < cmd.params.size()) {
					int limit = std::atoi(cmd.params[argIndex++].c_str());
					if (limit > 0) {
						channel->setUserLimit(limit);
					}
				}
				else {
					channel->removeUserLimit();
				}
				break;
		}
	}
}

void Server::_handleInvite(Client *client, const ParsedCommand &cmd) {
	std::string targetNick = cmd.params[0];
	std::string channelName = cmd.params[1];

	Client *targetClient = this->_getClientByNick(targetNick);

	if (!targetClient) {
		client->sendRaw(Replies::errNoSuchNickOrChannel(targetNick));
		return;
	}

	Channel *channel = this->_getChannelByName(channelName);

	if (!channel) {
		client->sendRaw(Replies::errNoSuchChannel(channelName));
		return;
	}

	if (!channel->hasMember(client->getFd())) {
		client->sendRaw(Replies::errNotOnChannel(channelName));
		return;
	}

	if (!channel->isOperator(client->getFd())) {
		client->sendRaw(Replies::errNotChannelOp(channelName));
		return;
	}

	if (channel->hasMember(targetClient->getFd())) {
		client->sendRaw(Replies::errAlreadyOnChannel(targetNick, channelName));
		return;
	}

	if (!channel->isInvited(targetClient)) {
		channel->invite(targetClient);
	}

	// Notify target
	targetClient->sendRaw(Replies::invite(client->getPrefix(), targetNick, channelName));

	// Confirm to inviter
	client->sendRaw(Replies::rplInviting(client->getNick(), targetNick, channelName));
}

void Server::_handleShow(Client *client, const ParsedCommand &cmd) {
	(void)client;
	if (cmd.params[0] == "server") {
		DEBUG_MSG(this->debugString());
	}
	if (cmd.params[0] == "client") {
		DEBUG_MSG(client->debugString());
	}
	if (cmd.params.size() == 2 && cmd.params[0] == "channel") {
		Channel* ch = this->_getChannelByName(cmd.params[1]);
		if (ch) {
			DEBUG_MSG(ch->debugString());
		}
	}
}

void Server::_handleTopic(Client *client, const ParsedCommand &cmd) {
	Channel *channel = this->_getChannelByName(cmd.params[0]);

	if (!channel) {
		client->sendRaw(Replies::noSuchChannel(client->getNick(), cmd.params[0]));
		return;
	}

	if (!channel->hasMember(client->getFd())) {
		client->sendRaw(Replies::notOnChannel(client->getNick(), channel->getName()));
		return;
	}

	std::string chanName = channel->getName();

	if (cmd.trailing.empty()) {
		if (!channel->hasTopic()) {
			client->sendRaw(Replies::noTopic(client->getNick(), chanName));
			return;
		}

		client->sendRaw(Replies::topic(client->getNick(), chanName, channel->getTopic()));
		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(client->getFd())) {
		client->sendRaw(Replies::notChannelOperator(client->getNick(), chanName));
		return;
	}

	channel->setTopic(cmd.trailing);
	channel->setTopicSetter(client->getNick());

	channel->broadcast(Replies::topicChanged(client->getPrefix(), chanName, cmd.trailing), -1);
}

void Server::_handleWho(Client *client, const ParsedCommand &cmd) {
	Channel *channel = this->_getChannelByName(cmd.params[0]);

	if (!channel) {
		client->sendRaw(Replies::noSuchChannel(client->getNick(), cmd.params[0]));
		return;
	}

	std::map<int, Client *> members = channel->getMembers();

	for (std::map<int, Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *target = it->second;

		client->sendRaw(Replies::whoReply(client->getNick(), channel->getName(),
				target->getUser(), target->getNick(), target->getRealname()));
	}

	client->sendRaw(Replies::endOfWho(client->getNick(), channel->getName()));
}