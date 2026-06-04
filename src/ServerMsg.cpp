#include "Server.hpp"

// Sends a message to a channel (all members except sender) or directly to a nick.
void Server::_handlePrivmsg(Client *client, const std::string &target, const std::string &trailing) {
	if (target.empty()) {
		client->sendRaw(Replies::errNoRecipient(client->getNick()));
		return;
	}

	if (trailing.empty()) {
		client->sendRaw(Replies::errNoTextToSend(client->getNick()));
		return;
	}

	std::string text = trailing;
	if (!text.empty() && text[0] == ':') {
		text = text.substr(1);
	}

	std::string prefix = ":" + client->getPrefix();

	std::string msg = Replies::privmsg(prefix, target, text);

	if (target[0] == '#') {
		std::map<std::string, Channel*>::iterator chIt = this->_channels.find(target);

		if (chIt == this->_channels.end()) {
			client->sendRaw( Replies::errNoSuchChannel(client->getNick(), target));
			return;
		}

		Channel *chan = chIt->second;

		if (!chan->hasMember(client->getFd())) {
			client->sendRaw(Replies::errNotOnChannel(client->getNick(), target));
			return;
		}

		const std::map<int, Client*> &members = chan->getMembers();

		for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
			if (it->first != client->getFd()) {
				it->second->sendRaw(msg);
			}
		}
	}
	else {
		for (std::map<int, Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
			if (it->second->getNick() == target) {
				it->second->sendRaw(msg);
				return;
			}
		}
		client->sendRaw(Replies::errNoSuchNick(client->getNick(), target));
	}
}


