#include "includes/Server.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

int main(int argc, char **argv) {
	if (argc != 3)	{
		std::cerr << "Error: Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	std::string checkStr(argv[1]);
	for (size_t i = 0; i < checkStr.size(); ++i) {
		if (!std::isdigit(checkStr[i])) {
			std::cerr << "Error: Invalid char in port" << std::endl;
			return 1;
		}
	}
	int port = std::atoi(argv[1]);
	if (port < 1024 || port > 65535) {
		std::cerr << "Error: Invalid port" << std::endl;
		return 1;
	}

	if (std::strlen(argv[2]) == 0) {
		std::cerr << "Error: Password cannot be empty" << std::endl;
		return 1;
	}
	std::string pass(argv[2]);
	if (pass.find(' ') != pass.npos) {
		std::cerr << "Error: Password cannot contain spaces" << std::endl;
		return 1;
	}

	try {
		Server server(port, pass);
		server.start();
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}