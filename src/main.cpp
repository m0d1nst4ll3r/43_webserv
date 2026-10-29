#include "WebServer.hpp"
#include "Color.hpp"
#include <string>
#include <iostream>

int	main(int argc, char **argv)
{
	(void)argc;
	try
	{
		WebServer webServ(std::string(argc > 1 ? argv[1] : ""));
		webServ.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << Color::RED << "Critical Error" << Color::RESET << ": " << e.what() << "\nServer shutting down...\n";
		return 1;
	}	
	return 0;
}