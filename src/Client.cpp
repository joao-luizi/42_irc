#include "Client.hpp"

Client::Client(int fd)
	: _fd(fd), _passOk(false), _nickOk(false), _userOk(false), _welcomed(false), _shouldDisconnect(false) {
}

Client::~Client(){
}

// Returns the client's socket file descriptor.
int	Client::getFd() const {
	return this->_fd;
}

// Returns the client's current nickname.
std::string	Client::getNick() const {
	return this->_nick;
}

// Returns the client's username.
std::string	Client::getUser() const {
	return this->_user;
}

// Sets the nickname and marks nickOk as true.
void	Client::setNick(const std::string newNick) {
	this->_nick = newNick;
}

// Sets the username and marks userOk as true.
void	Client::setUser(const std::string newUser) {
	this->_user = newUser;
	this->_userOk = true;
}

// Marks passOk as true. Called when PASS matches the server password.
void	Client::setPassOk() {
	this->_passOk = true;
}

// Marks nickOK as true. Called when NICK is unique at the server level.
void	Client::setNickOk() {
	this->_nickOk = true;
}

// Returns true when passOk, nickOk and userOk are all set.
bool	Client::isRegistered() const {
	return this->_passOk && this->_nickOk && this->_userOk;
}

// Appends raw bytes from recv() to the input buffer.
void	Client::appendBuffer(const std::string &data) {
	this->_inputBuffer += data;
}

// Returns true if the buffer contains a complete line ending in \r\n.
bool	Client::hasLine() const {
	return this->_inputBuffer.find("\r\n") != std::string::npos;
}

bool	Client::hasPassOk() const {
	return this->_passOk == true;
}

// Extracts and removes the first complete line from the buffer (strips \r\n).
std::string	Client::getLine() {
	size_t		pos = this->_inputBuffer.find("\r\n");
	std::string	line = this->_inputBuffer.substr(0, pos);
	this->_inputBuffer.erase(0, pos + 2);
	return line;
}

// Sends a raw IRC-formatted message to this client via its socket fd.
void	Client::sendRaw(const std::string &msg) const {
	::send(this->_fd, msg.c_str(), msg.size(), 0);
}

// Returns true if the 001 welcome message has already been sent.
bool	Client::isWelcomed() const {
	return this->_welcomed;
}

// Marks the client as welcomed. Called once, immediately after sending 001.
void	Client::setWelcomed() {
	this->_welcomed = true;
}

const	std::set<std::string>& Client::getChannels() const {
	 return this->_channels;
}

void	Client::addChannel(const std::string &channelName) {
		this->_channels.insert(channelName);
}

void	Client::removeChannel(const std::string &channelName) {
		this->_channels.erase(channelName);
}

void	Client::setRealName(const std::string &realName) {
	this->_realname = realName;
}

bool	Client::isToDisconnect() const {
		return this->_shouldDisconnect;
}

void	Client::setForDisconnect() {
		this->_shouldDisconnect = true;
}

std::string Client::getPrefix() const {
	return _nick + "!" + _user + "@localhost";
}

std::string	Client::getRealname() const {
	return _realname;
}

//Debug
std::string	Client::debugString() const {
	std::stringstream					ss;
	std::set<std::string>::const_iterator	it;

	ss << "=== CLIENT ===" << std::endl;

	ss << "FD: " << _fd << std::endl;

	ss << "Nick: ";
	if (_nick.empty()) {
		ss << "(none)";
	}
	else {
		ss << _nick;
	}

	ss << std::endl;

	ss << "User: ";
	if (_user.empty()) {
		ss << "(none)";
	}
	else {
		ss << _user;
	}
	ss << std::endl;

	ss << "Realname: ";
	if (_realname.empty()) {
		ss << "(none)";
	}
	else {
		ss << _realname;
	}
	ss << std::endl;

	ss << "Pass OK: ";
	if (_passOk) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Nick OK: ";
	if (_nickOk) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "User OK: ";
	if (_userOk) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Welcomed: ";
	if (_welcomed) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Should Disconnect: ";
	if (_shouldDisconnect) {
		ss << "yes";
	}
	else {
		ss << "no";
	}
	ss << std::endl;

	ss << "Input Buffer Size: " << _inputBuffer.size() << std::endl;

	ss << "Channels (" << _channels.size() << "): ";
	for (it = _channels.begin(); it != _channels.end(); ++it) {
		ss << *it;

		std::set<std::string>::const_iterator next = it;
		++next;

		if (next != _channels.end()) {
			ss << ", ";
		}
	}
	ss << std::endl;

	return (ss.str());
}

