#include "WebServer.hpp"
#include "Color.hpp"
#include <string>
#include <iostream>

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << Color::CYAN << "Usage" << Color::RESET << ":\n    ./webserv config_file\n";
		return 2;
	}
	try
	{
		WebServer webServ(std::string(argc > 1 ? argv[1] : ""));
		webServ.openListenSockets();
		webServ.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << Color::RED << "Critical Error" << Color::RESET << ": " << e.what() << "\nServer shutting down...\n";
		return 1;
	}	
	return 0;
}