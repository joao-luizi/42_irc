#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>

#include "Client.hpp"

class Channel {
	private:
		std::string				_name;
		std::map<int, Client*>	_members;
		std::set<int>			_operators;
		bool 					_inviteOnly;
		bool 					_topicRestricted;
		std::string 			_key;
		int 					_userLimit;
		std::set<Client *> 		_invited;
		std::string 			_topic;
		std::string 			_topicSetter;

		Channel();
		Channel(const Channel &other);
		Channel &operator=(const Channel &other);

	public:
		Channel(const std::string &name);
		~Channel();

		const std::string				&getName() const;

		void							setTopic(const std::string &topic);
		void							setTopicSetter(const std::string &nick);
		bool							hasTopic() const;
		const std::string 				&getTopic() const;
		bool 							isTopicRestricted() const;
		void 							setTopicRestricted(bool val);
		void							addMember(Client *client);
		bool							hasMember(int fd) const;
		bool							hasKey() const;
		bool							isOperator(int fd) const;
		void							removeMember(Client *client);

		bool 							isInviteOnly() const;
		int  							getUserLimit() const;
		std::string						getMemberList() const;
		std::string						getKey() const;
		void 							broadcast(const std::string &msg, int excludeFd);
		const std::map<int, Client*>	&getMembers() const;

		void 							setInviteOnly(bool val);
		void 							setKey(const std::string& key);
		void 							removeKey();
		bool 							setOperator(Client *client, bool addFd);
		void 							setUserLimit(int limit);
		void 							removeUserLimit();

		std::string 					getModes() const;
		void 							invite(Client *client);
		bool 							isInvited(Client *client) const;
		void 							removeInvite(Client *client);

		std::string 					debugString() const;
};

#endif