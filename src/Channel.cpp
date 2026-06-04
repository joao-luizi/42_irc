#include "Channel.hpp"

Channel::Channel(const std::string &name)
	: _name(name), _inviteOnly(false), _topicRestricted(false),
	 _key(""), _userLimit(-1), _topic("") {
}

Channel::~Channel() {
}

// Returns the channel name.
const std::string &Channel::getName() const {
	return this->_name;
}

// Adds client to _members.
void Channel::addMember(Client *client) {
	this->_members[client->getFd()] = client;
}

// Returns true if fd is present in _members.
bool Channel::hasMember(int fd) const {
	return this->_members.find(fd) != this->_members.end();
}
//returns true id fd is present in _operators
bool Channel::isOperator(int fd) const {
	return this->_operators.find(fd) != this->_operators.end();
}

bool	Channel::hasKey() const {
	return !_key.empty();
}

// Removes client based on its fd.
// if no Op left, promote next member
void Channel::removeMember(Client *client) 
{
	this->_members.erase(client->getFd());
	this->_operators.erase(client->getFd());

	if (_operators.empty() && !_members.empty()) {
		_operators.insert(_members.begin()->first);
	}
}

// Returns a space-separated list of member nicks. Ops are prefixed with @.
std::string Channel::getMemberList() const {
	std::string list;
	std::map<int, Client*>::const_iterator it = this->_members.begin();
	for (; it != this->_members.end(); ++it) {
		if (!list.empty()){
			list += " ";
		}
		if (this->_operators.count(it->first)){
			list += "@";
		}
		list += it->second->getNick();
	}
	return list;
}
std::string	Channel::getKey() const {
	return _key;
}

// Sends msg to every member in the channel.
void Channel::broadcast(const std::string &msg, int excludeFd) {
	for (std::map<int, Client*>::iterator it = _members.begin(); it != _members.end(); ++it) {
		if (it->first != excludeFd){
			it->second->sendRaw(msg);
		}
	}
}

// Returns a const reference to the _members map. Used by Server for PRIVMSG iteration
const std::map<int, Client*> &Channel::getMembers() const {
	return this->_members;
}

void Channel::setInviteOnly(bool val) {
	this->_inviteOnly = val;
}

void Channel::setTopicRestricted(bool val) {
	this->_topicRestricted = val;
}

bool Channel::isTopicRestricted() const {
	return this->_topicRestricted;
}

const std::string &Channel::getTopic() const {
	return this->_topic;
}

bool Channel::hasTopic() const {
	return !this->_topic.empty();
}

void Channel::setTopicSetter(const std::string &nick) {
	_topicSetter = nick;
}

void Channel::setTopic(const std::string &topic) {
	_topic = topic;
}

void Channel::setKey(const std::string& key) {
	this->_key = key;
}

void Channel::removeKey() {
	this->_key.erase();
}

bool Channel::isInviteOnly() const {
	return _inviteOnly;
}

int  Channel::getUserLimit() const {
	return _userLimit;
}

bool Channel::setOperator(Client *client, bool add) {
	if (add) {
		if (isOperator(client->getFd())) {
			return false;
		}
		_operators.insert(client->getFd());
		return true;
	}
	else {
		if (!isOperator(client->getFd())) {
			return false;
		}
		_operators.erase(client->getFd());
		return true;
	}
}

void Channel::setUserLimit(int limit) {
	this->_userLimit = limit;
}

void Channel::removeUserLimit() {
	this->_userLimit = -1;
}

std::string Channel::getModes() const {
	std::string modes;

	if (_inviteOnly) {
		modes += "i";
	}
	if (_topicRestricted) {
		modes += "t";
	}
	if (!_key.empty()) {
		modes += "k";
	}
	if (_userLimit != -1) {
		modes += "l";
	}

	return modes;
}

void Channel::invite(Client *client) {
	_invited.insert(client);
}

bool Channel::isInvited(Client *client) const {
	return (_invited.count(client) != 0);
}

void Channel::removeInvite(Client *client) {
	_invited.erase(client);
}

std::string	Channel::debugString() const {
	std::stringstream						ss;
	std::map<int, Client*>::const_iterator	memberIt;
	std::set<int>::const_iterator			opIt;

	ss << "=== CHANNEL ===" << std::endl;

	ss << "Name: " << _name << std::endl;

	ss << "Members (" << _members.size() << "): ";
	for (memberIt = _members.begin(); memberIt != _members.end(); ++memberIt) {
		ss << memberIt->second->getPrefix();

		std::map<int, Client*>::const_iterator next = memberIt;
		++next;

		if (next != _members.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	ss << "Operators (" << _operators.size() << "): ";
	for (opIt = _operators.begin(); opIt != _operators.end(); ++opIt) {
		std::map<int, Client*>::const_iterator member = _members.find(*opIt);

		if (member != _members.end()) {
			ss << member->second->getPrefix();
		}
		else {
			ss << "(unknown fd " << *opIt << ")";
		}

		std::set<int>::const_iterator next = opIt;
		++next;

		if (next != _operators.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	ss << "Invite Only: ";
	if (_inviteOnly) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Topic Restricted: ";
	if (_topicRestricted) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Key: ";
	if (_key.empty()) {
		ss << "(none)";
	}
	else {
		ss << _key;
	}
	ss << std::endl;

	ss << "User Limit: ";
	if (_userLimit < 0) {
		ss << "(none)";
	}
	else {
		ss << _userLimit;
	}
	ss << std::endl;

	return (ss.str());
}

