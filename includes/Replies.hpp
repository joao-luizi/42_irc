#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

namespace Replies
{
	// Registration / auth
	std::string welcome(const std::string& nick);
	std::string needMoreParams(const std::string& nick, const std::string& cmd);
	std::string notRegistered(void);
	std::string errPassMismatch(void);

	// Nick handling
	std::string nickInUse(const std::string& nick);
	std::string noNickGiven(void);

	// Generic errors
	std::string unknownCommand(const std::string& cmd);

	// IRC message formatting
	std::string nickChange(const std::string& oldNick, const std::string& user, const std::string& newNick);

	// JOIN errors
	std::string errNoSuchChannel(const std::string& nick, const std::string& channel);
	std::string errChannelFull(const std::string& nick, const std::string& channel);
	std::string errInviteOnly(const std::string& nick, const std::string& channel);
	std::string errBadChannelKey(const std::string& nick, const std::string& channel);
	std::string errUserOnChannel(const std::string &nick, const std::string &channel);

	// JOIN success / info
	std::string rplJoin(const std::string& prefix, const std::string& channel);
	std::string rplNamReply(const std::string& nick, const std::string& channel, const std::string& names);
	std::string rplEndOfNames(const std::string& nick, const std::string& channel);

	// MODE errors
	std::string errInvalidChannelName(const std::string& nick, const std::string& channel);
	std::string errNoSuchChannel(const std::string& nick, const std::string& channel);
	std::string errNotOnChannel(const std::string& nick, const std::string& channel);
	std::string errNotChannelOp(const std::string& nick, const std::string& channel);

	// MODE reply (query)
	std::string modeReply(const std::string& nick, const std::string& channel, const std::string& modes);
	// MODE message
	std::string modeOp(const std::string& prefix, const std::string& channel, const std::string& nick);
	// MODE Change
	std::string modeChange(const std::string& prefix, const std::string& channel, const std::string& mode, const std::string& target);

	// PRIVMSG errors
	std::string errNoRecipient(const std::string& nick);
	std::string errNoTextToSend(const std::string& nick);
	std::string errNoSuchChannel(const std::string& nick, const std::string& channel);
	std::string errNoSuchNick(const std::string& nick, const std::string& target);

	// PRIVMSG format
	std::string privmsg(const std::string& prefix, const std::string& target, const std::string& text);

	std::string quit(const std::string& prefix, const std::string& reason);

	// KICK errors
	std::string errNoSuchChannel(const std::string& nick, const std::string& channel);
	std::string errNotOnChannel(const std::string& nick, const std::string& channel);
	std::string errNotChannelOp(const std::string& nick, const std::string& channel);
	std::string errNoSuchNick(const std::string& nick, const std::string& target);
	std::string errUserNotOnChannel(const std::string& nick, const std::string& target, const std::string& channel);

	// KICK message
	std::string kick(const std::string& prefix, const std::string& channel, const std::string& target, const std::string& reason);

	// INVITE errors
	std::string errNoSuchNickOrChannel(const std::string& target);
	std::string errNoSuchChannel(const std::string& channel);
	std::string errNotOnChannel(const std::string& channel);
	std::string errNotChannelOp(const std::string& channel);
	std::string errAlreadyOnChannel(const std::string& nick, const std::string& channel);

	// INVITE success
	std::string invite(const std::string& prefix, const std::string& targetNick, const std::string& channel);
	std::string rplInviting(const std::string& inviter, const std::string& targetNick, const std::string& channel);

	std::string noSuchChannel(const std::string& nick, const std::string& channel);
	std::string notOnChannel(const std::string& nick, const std::string& channel);
	std::string noTopic(const std::string& nick, const std::string& channel);
	std::string topic(const std::string& nick, const std::string& channel, const std::string& topic);
	std::string notChannelOperator(const std::string& nick, const std::string& channel);

	std::string errTooManyParams(const std::string &nick, const std::string &cmd);
	std::string errAlreadyRegistered(const std::string &nick);
	std::string erroneousNick(const std::string &nick);
	// Commands
	std::string topicChanged(const std::string& prefix, const std::string& channel, const std::string& topic);

	std::string whoReply(const std::string& requesterNick, const std::string& channel,
		const std::string& username, const std::string& nick, const std::string& realname);

	std::string errInvalidParams(const std::string &nick, const std::string &command);
	std::string rplWelcome(const std::string &nick);
	std::string rplYourHost(const std::string &nick);
	std::string rplCreated(const std::string &nick);
	std::string rplMyInfo(const std::string &nick);
	std::string rplTopic(const std::string &nick, const std::string &channel, const std::string &topic);
	std::string endOfWho(const std::string& nick, const std::string& channel);

}

#endif