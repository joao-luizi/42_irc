#include "Server.hpp"

// Creates channel if needed, adds client as member (op if first), broadcasts JOIN, sends 353/366.
void Server::_handleJoin(Client *client, const ParsedCommand &cmd) {
	std::string chanName = cmd.params[0];

	size_t space = chanName.find(' ');
	if (space != std::string::npos)
		chanName = chanName.substr(0, space);

	if (chanName.empty() || chanName[0] != '#' || chanName.size() > 200	||
	 chanName.find(' ') != std::string::npos || chanName.find(',') != std::string::npos	||
	 chanName.find('\007') != std::string::npos) {
		client->sendRaw(Replies::errNoSuchChannel(client->getNick(), chanName));
		return;
	}

	if (this->_channels.find(chanName) == this->_channels.end()) {
		this->_channels[chanName] = new Channel(chanName);
	}

	Channel *chan = this->_channels[chanName];

	if (chan->hasMember(client->getFd())) {
		client->sendRaw(Replies::errUserOnChannel(client->getNick(),chan->getName()));
		return;
	}

	if (chan->getUserLimit() > 0 && (int)chan->getMembers().size() >= chan->getUserLimit()) {
		client->sendRaw(Replies::errChannelFull(client->getNick(), chanName));
		return;
	}

	if (chan->isInviteOnly() && !chan->isInvited(client)) {
		client->sendRaw(Replies::errInviteOnly(client->getNick(), chanName));
		return;
	}

	if (chan->hasKey()) {
		if (cmd.params.size() < 2 || cmd.params[1].empty() ||
		 chan->getKey() != cmd.params[1]) {
			client->sendRaw(Replies::errBadChannelKey(client->getNick(), chanName));
			return;
		}
	}

	bool isFirstMember = chan->getMembers().empty();

	chan->addMember(client);
	chan->removeInvite(client);
	client->addChannel(chan->getName());
	if (chan->hasTopic()) {
		client->sendRaw(Replies::rplTopic(client->getNick(), chanName, chan->getTopic()));
	}

	chan->broadcast(Replies::rplJoin(client->getPrefix(), chanName),-1);

	client->sendRaw(Replies::rplNamReply(client->getNick(), chanName, chan->getMemberList()));

	client->sendRaw(Replies::rplEndOfNames(client->getNick(), chanName));

	if (isFirstMember && chan->setOperator(client, true)) {
		chan->broadcast(Replies::modeOp(client->getPrefix(), chanName, client->getNick()),-1);
	}
}

void Server::_handlePart(Client *client, const ParsedCommand &cmd) {
	std::string chanName = cmd.params[0];

	std::map<std::string, Channel*>::iterator it = this->_channels.find(chanName);
	if (it == this->_channels.end()) {
		client->sendRaw(Replies::errNoSuchChannel(client->getNick(), chanName));
		return;
	}

	Channel *channel = it->second;

	if (!channel->hasMember(client->getFd())) {
		client->sendRaw(Replies::errNotOnChannel(client->getNick(), chanName));
		return;
	}

	std::string reason = cmd.trailing.empty() ? "" : " :" + cmd.trailing;

	std::string msg = ":" + client->getPrefix() + " PART " + chanName + reason + "\r\n";

	channel->broadcast(msg, -1);

	channel->removeMember(client);
	client->removeChannel(chanName);
}