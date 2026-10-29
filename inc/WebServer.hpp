#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "ServerConfig.hpp"
#include "EpollInstance.hpp"
#include "IOChannel.hpp"
#include "ListenSocket.hpp"
#include <string>
#include <vector>

class ListenSocket;

class WebServer
{
	public:

	WebServer(const std::string& configFile);
	~WebServer();

	// Run server loop
	void	run();

	void	addNewClient(IOChannel* client);
	void	removeClient();
	void	addNewCGI(IOChannel* cgi);

	private:

	const ServerConfig				_serverConfig;
	//WebsiteConfig					_websiteConfig; // WIP - No Config yet
	EpollInstance					_epoll;
	std::vector<IOChannel*>			_ioChannels;
	std::vector<ListenSocket*>		_lsocks;
	unsigned int					_numClients;
	bool							_listening;
};

#endif /* WEBSERVER_HPP */