#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <iostream>


enum CommandType {
	PASS, NICK, USER, JOIN, PART, QUIT, SHOW,
	KICK, INVITE, TOPIC, MODE, PRIVMSG, WHO,
	CAP, UNKNOWN
};

struct ParsedCommand {
	CommandType type;
	std::string command;
	std::vector<std::string> params;
	std::string trailing;
};


class Parser {
	private:
		static void _toCommandType(const std::string& line, size_t start, size_t last, ParsedCommand &out);
		static void _extractCommand(const std::string& line, ParsedCommand& out, size_t& pos);
		static void _extractParamsAndTrailing(const std::string& line, ParsedCommand& out, size_t pos);
		static void _splitParams(const std::string& segment, ParsedCommand& out);
	public:
		static ParsedCommand parse(const std::string& line);
		static std::string commandTypeToString(CommandType type);
		static bool hasControlChars(const std::string &s);
		
	
};

std::ostream &operator<<(std::ostream &os, const ParsedCommand &cmd);


#endif