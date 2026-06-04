#include "Replies.hpp"

static const std::string SERVER_NAME = "Awesome.42.ft.irc";

static std::string prefix() {
	return ":" + SERVER_NAME + " ";
}


std::string Replies::errPassMismatch(void) {
	return prefix() + "464 * :Password incorrect\r\n";
}

// -------------------- WELCOME --------------------

std::string Replies::welcome(const std::string& nick) {
	return prefix() + "001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n";
}

// -------------------- ERRORS --------------------

std::string Replies::needMoreParams(const std::string& nick, const std::string& cmd) {
	std::string n = nick.empty() ? "*" : nick;
	return prefix() + "461 " + n + " " + cmd + " :Not enough parameters\r\n";
}

std::string Replies::notRegistered(void) {
	return prefix() + "451 * :You have not registered\r\n";
}

std::string Replies::nickInUse(const std::string& nick) {
	return prefix() + "433 * " + nick + " :Nickname is already in use\r\n";
}

std::string Replies::noNickGiven(void) {
	return prefix() + "431 * :No nickname given\r\n";
}

std::string Replies::unknownCommand(const std::string& cmd) {
	return prefix() + "421 " + cmd + " :Unknown command\r\n";
}

std::string Replies::errNoSuchChannel(const std::string& nick, const std::string& channel) {
	return prefix() + "403 " + nick + " " + channel + " :Invalid channel name\r\n";
}

std::string Replies::errChannelFull(const std::string& nick, const std::string& channel) {
	return prefix() + "471 " + nick + " " + channel + " :Cannot join channel (+l)\r\n";
}

std::string Replies::errInviteOnly(const std::string& nick, const std::string& channel) {
	return prefix() + "473 " + nick + " " + channel + " :Cannot join channel (+i)\r\n";
}

std::string Replies::errBadChannelKey(const std::string& nick, const std::string& channel) {
	return prefix() + "475 " + nick + " " + channel + " :Cannot join channel (+k)\r\n";
}

std::string Replies::rplJoin(const std::string& prefix, const std::string& channel) {
	return ":" + prefix + " JOIN " + channel + "\r\n";
}

std::string Replies::rplNamReply(const std::string& nick, const std::string& channel, const std::string& names){
	return prefix() + "353 " + nick + " = " + channel + " :" + names + "\r\n";
}

std::string Replies::rplEndOfNames(const std::string& nick, const std::string& channel) {
	return prefix() + "366 " + nick + " " + channel + " :End of /NAMES list\r\n";
}


std::string Replies::errNoRecipient(const std::string& nick) {
	return prefix() + "411 " + nick + " :No recipient given\r\n";
}

std::string Replies::errNoTextToSend(const std::string& nick) {
	return prefix() + "412 " + nick + " :No text to send\r\n";
}



std::string Replies::errNoSuchNick(const std::string& nick, const std::string& target) {
	return prefix() + "401 " + nick + " " + target + " :No such nick\r\n";
}

std::string Replies::quit(const std::string& prefix, const std::string& reason) {
	return ":" + prefix + " QUIT :" + reason + "\r\n";
}


std::string Replies::errNotOnChannel(const std::string& nick, const std::string& channel) {
	return prefix() + "442 " + nick + " " + channel + " :You're not on that channel\r\n";
}


std::string Replies::errUserNotOnChannel(const std::string& nick, const std::string& target, const std::string& channel) {
	return prefix() + "441 " + nick + " " + target + " " + channel + " :They aren't on that channel\r\n";
}

std::string Replies::errInvalidChannelName(const std::string& nick, const std::string& channel) {
	return prefix() + "476 " + nick + " " + channel + " :Invalid channel name\r\n";
}


std::string Replies::errNotChannelOp(const std::string& nick, const std::string& channel) {
	return prefix() + "482 " + nick + " " + channel + " :You're not channel operator\r\n";
}

std::string Replies::errNoSuchNickOrChannel(const std::string& target) {
	return prefix() + "401 " + target + " :No such nick/channel\r\n";
}

std::string Replies::errNoSuchChannel(const std::string& channel) {
	return prefix() + "403 " + channel + " :No such channel\r\n";
}

std::string Replies::errNotOnChannel(const std::string& channel) {
	return prefix() + "442 " + channel + " :You're not on that channel\r\n";
}

std::string Replies::errNotChannelOp(const std::string& channel) {
	return prefix() + "482 " + channel + " :You're not channel operator\r\n";
}

std::string Replies::errAlreadyOnChannel(const std::string& nick, const std::string& channel) {
	return prefix() + "443 " + nick + " " + channel + " :is already on channel\r\n";
}

// Replies.cpp

std::string Replies::noSuchChannel(const std::string& nick, const std::string& channel) {
	return prefix() + "403 " + nick + " " + channel + " :No such channel\r\n";
}

std::string Replies::notOnChannel(const std::string& nick, const std::string& channel) {
	return prefix() + "442 " + nick + " " + channel + " :You're not on that channel\r\n";
}

std::string Replies::noTopic(const std::string& nick, const std::string& channel) {
	return prefix() + "331 " + nick + " " + channel + " :No topic is set\r\n";
}

std::string Replies::topic(const std::string& nick, const std::string& channel, const std::string& topic) {
	return prefix() + "332 " + nick + " " + channel + " :" + topic + "\r\n";
}

std::string Replies::notChannelOperator(const std::string& nick, const std::string& channel) {
	return prefix() + "482 " + nick + " " + channel + " :You're not channel operator\r\n";
}

// Replies.cpp

std::string Replies::whoReply(const std::string& requesterNick, const std::string& channel, const std::string& username,
	const std::string& nick, const std::string& realname) {
	return prefix() + "352 " + requesterNick + " " + channel + " " + username + " "
		+ "localhost " + SERVER_NAME + " " + nick + " H :0 " + realname + "\r\n";
}

std::string Replies::endOfWho(const std::string& nick, const std::string& channel) {
	return prefix() + "315 " + nick + " " + channel + " :End of /WHO list\r\n";
}

std::string Replies::errTooManyParams(const std::string &nick, const std::string &cmd) {
	std::string n = nick.empty() ? "*" : nick;
	return prefix() + "461 " + n + " " + cmd + " :Too many parameters\r\n";
}

std::string Replies::errAlreadyRegistered(const std::string &nick) {
	return prefix() + "462 " + nick + " :You may not reregister\r\n";
}

std::string Replies::erroneousNick(const std::string &nick) {
	std::string n = nick.empty() ? "*" : nick;
	return (prefix() + "432 * " + n + " :Erroneous nickname\r\n");
}

std::string Replies::errUserOnChannel(const std::string &nick, const std::string &channel) {
	return prefix() + "443 " + nick + " " + channel +
		   " :is already on channel\r\n";
}

std::string Replies::errInvalidParams(const std::string &nick, const std::string &command) {
	return prefix() + "400 " + nick + " " + command + " :Invalid parameters\r\n";
}

// -------------------- MESSAGES --------------------

std::string Replies::rplTopic(const std::string &nick, const std::string &channel, const std::string &topic) {
	return prefix() + "332 " + nick + " " + channel + " :" + topic + "\r\n";
}

std::string Replies::rplWelcome(const std::string &nick) {
	return (prefix() + "001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
}

std::string Replies::rplYourHost(const std::string &nick) {
	return prefix() + "002 " + nick + " :Your host is " + SERVER_NAME + "\r\n";
}

std::string Replies::rplCreated(const std::string &nick) {
	return (prefix() + "003 " + nick + " :This server was created today\r\n");
}

std::string Replies::rplMyInfo(const std::string &nick) {
	return prefix() + "004 " + nick + " " + SERVER_NAME + " 1.0 o o\r\n";
}

std::string Replies::topicChanged(const std::string& prefix, const std::string& channel, const std::string& topic) {
	return ":" + prefix + " TOPIC " + channel + " :" + topic + "\r\n";
}

std::string Replies::invite(const std::string& prefix, const std::string& targetNick, const std::string& channel){
	return ":" + prefix + " INVITE " + targetNick + " :" + channel + "\r\n";
}

std::string Replies::rplInviting(const std::string& inviter, const std::string& targetNick, const std::string& channel) {
	return prefix() + "341 " + inviter + " " + targetNick + " " + channel + "\r\n";
}

std::string Replies::modeChange(const std::string& prefix, const std::string& channel, const std::string& mode, const std::string& target) {
	return ":" + prefix + " MODE " + channel + " " + mode + " " + target + "\r\n";
}

std::string Replies::modeReply(const std::string& nick, const std::string& channel, const std::string& modes) {
	return prefix() + "324 " + nick + " " + channel + " +" + modes + "\r\n";
}

std::string Replies::modeOp(const std::string& prefix, const std::string& channel, const std::string& nick) {
	return ":" + prefix + " MODE " + channel + " +o " + nick + "\r\n";
}

std::string Replies::kick( const std::string& prefix, const std::string& channel, const std::string& target, const std::string& reason) {
	return ":" + prefix + " KICK " + channel + " " + target + " :" + reason + "\r\n";
}

std::string Replies::privmsg(const std::string& prefix, const std::string& target, const std::string& text) {
	return prefix + " PRIVMSG " + target + " :" + text + "\r\n";
}


std::string Replies::nickChange(const std::string& oldNick, const std::string& user, const std::string& newNick) {
	return ":" + oldNick + "!" + user + "@localhost NICK :" + newNick + "\r\n";
}

