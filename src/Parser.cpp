#include "Parser.hpp"

bool Parser::hasControlChars(const std::string &s) {
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(s[i]);

		if ((c <= 31) || c == 127) {
			return true;
		}
	}
	return false;
}

void Parser::_extractCommand(const std::string& line, ParsedCommand& out, size_t& i) {
	while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
		i++;
	}

	size_t space = line.find(' ', i);

	if (space == std::string::npos) {
		_toCommandType(line, i, line.size(), out);
		i = line.size();
		return;
	}

	_toCommandType(line, i, space, out);

	i = space + 1;
}

void Parser::_extractParamsAndTrailing(const std::string& line, ParsedCommand& out, size_t i) {
	size_t trailingPos = std::string::npos;

	for (size_t j = i; j < line.size(); ++j) {
		if (line[j] == ':' && (j == i || line[j - 1] == ' ')) {
			trailingPos = j;
			break;
		}
	}
	
	std::string paramPart;

	if (trailingPos != std::string::npos) {
		paramPart = line.substr(i, trailingPos - i);
		out.trailing = line.substr(trailingPos + 1);
	}
	else {
		paramPart = line.substr(i);
	}

	_splitParams(paramPart, out);
}

void Parser::_splitParams(const std::string& segment, ParsedCommand& out) {
	size_t i = 0;

	while (i < segment.size()) {
		while (i < segment.size() && segment[i] == ' ') {
			i++;
		}

		if (i >= segment.size()) {
			break;
		}

		size_t start = i;

		while (i < segment.size() && segment[i] != ' ') {
			i++;
		}

		out.params.push_back(segment.substr(start, i - start));
	}
}

void Parser::_toCommandType(const std::string& line,
							size_t start,
							size_t last,
							ParsedCommand &out) {
	if (start >= last) {
		out.type = UNKNOWN;
		return;
	}

	std::string cmd(line.begin() + start, line.begin() + last);
	for (size_t i = 0; i < cmd.size(); ++i){
		cmd[i] = std::toupper(cmd[i]);
	}

	out.command = cmd; // used in error replies (461, 421, etc.)
	if (cmd == "PASS") { out.type = PASS; return; }
	if (cmd == "NICK") { out.type = NICK; return; }
	if (cmd == "USER") { out.type = USER; return; }
	if (cmd == "JOIN") { out.type = JOIN; return; }
	if (cmd == "PART") { out.type = PART; return; }
	if (cmd == "KICK") { out.type = KICK; return; }
	if (cmd == "INVITE") { out.type = INVITE; return; }
	if (cmd == "TOPIC") { out.type = TOPIC; return; }
	if (cmd == "MODE") { out.type = MODE; return; }
	if (cmd == "PRIVMSG") { out.type = PRIVMSG; return; }
	if (cmd == "QUIT") { out.type = QUIT; return; }
	if (cmd == "WHO") { out.type = WHO; return; }
	if (cmd == "CAP") { out.type = CAP; return; }
	#ifdef DEBUG
	if (cmd == "SHOW") { out.type = SHOW; return; }
	#endif

	out.type = UNKNOWN;
}

ParsedCommand Parser::parse(const std::string& line) {
	ParsedCommand out;
	size_t i = 0;

	_extractCommand(line, out, i);
	_extractParamsAndTrailing(line, out, i);

	return out;
}

std::string Parser::commandTypeToString(CommandType type) {
	switch (type) {
		case PASS: return "PASS";
		case NICK: return "NICK";
		case USER: return "USER";
		case JOIN: return "JOIN";
		case PART: return "PART";
		case KICK: return "KICK";
		case INVITE: return "INVITE";
		case TOPIC: return "TOPIC";
		case MODE: return "MODE";
		case QUIT: return "QUIT";
		case PRIVMSG: return "PRIVMSG";
		case SHOW: return "SHOW";
		case WHO: return "WHO";
		default: return "UNKNOWN";
	}
};

std::ostream &operator<<(std::ostream &os, const ParsedCommand &cmd) {
	os << "ParsedCommand {\n";
	os << "  type: " << Parser::commandTypeToString(cmd.type) << "\n";
	os << "  command: " << cmd.command << "\n";

	os << "  params: [";

	std::vector<std::string>::const_iterator it = cmd.params.begin();
	while (it != cmd.params.end()) {
		os << *it;
		++it;
		if (it != cmd.params.end()) {
			os << ", ";
		}
	}

	os << "]\n";

	os << "  trailing: " << cmd.trailing << "\n";
	os << "}\n";

	return os;
}