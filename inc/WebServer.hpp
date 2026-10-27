#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "ServerConfig.hpp"
#include "EpollInstance.hpp"
#include "ListenSocket.hpp"
#include "ClientSession.hpp"
#include <string>
#include <map>

class WebServer
{
	public:

	WebServer(const std::string& configFile);
	~WebServer();

	// Run server loop
	void	run();

	private:

	ServerConfig					_serverConfig;
	//WebsiteConfig					_websiteConfig; // WIP - No Config yet
	EpollInstance					_epoll;
	std::map<int, ListenSocket*>	_listenSockets;
	std::map<int, ClientSession*>	_clientSessions;
	std::map<int, ClientSession*>	_cgiPipesIn;
	std::map<int, ClientSession*>	_cgiPipesOut;
};

#endif /* WEBSERVER_HPP */