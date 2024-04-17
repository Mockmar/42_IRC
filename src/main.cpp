#include <iostream>
#include "Server.hpp"
#include <cstring>

int	main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		exit(1);
	}

	if (!Server::is_valid_port(argv[1])) {
		std::cerr << "Invalid port number\n";
		exit(2);
	}
	if (!Server::is_valid_password(argv[2])) {
		std::cerr << "Invalid password\n";
		exit(3);
	}

	Server	server(argv[1], argv[2]);
	if (!server.start())
		server.poll();

	return (0);
}
