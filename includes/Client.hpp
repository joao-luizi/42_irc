#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <sys/socket.h>
#include <sstream>
class Client {
	private:
		int			_fd;
		std::string	_inputBuffer;
		std::string	_nick;
		std::string	_user;
		std::string	_realname;
		bool		_passOk;
		bool		_nickOk;
		bool		_userOk;
		bool		_welcomed;
		bool 		_shouldDisconnect;
		std::set<std::string> _channels;

		Client();
		Client(const Client &other);
		Client &operator=(const Client &other);

	public:
		Client(int fd);
		~Client();

		int			getFd() const;
		bool		isRegistered() const;
		bool 		isToDisconnect() const;
		bool		hasPassOk() const;
		void 		setForDisconnect();
		void		setNick(const std::string newNick);
		void		setUser(const std::string newUser);
		void		setPassOk();
		void		setNickOk();
		void		setWelcomed();
		void		addChannel(const std::string &channelName);
		void		removeChannel(const std::string &channelName);
		void 		setRealName(const std::string &realName);


		std::string	getNick() const;
		std::string	getUser() const;
		std::string getPrefix() const;
		std::string getRealname() const;

		bool		isWelcomed() const;


		const std::set<std::string>& getChannels() const;


		void		appendBuffer(const std::string &data);
		bool		hasLine() const;
		std::string	getLine();

		void		sendRaw(const std::string &msg) const;

		//Debug
		std::string debugString() const;
};

#endif