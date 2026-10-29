#include "WebServer.hpp"
#include "Color.hpp"
#include "EpollInstance.hpp"
#include "IOChannel.hpp"
#include "ListenSocket.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <csignal>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

namespace
{
	volatile sig_atomic_t gServerRunning = 1;

	void	handleShutdownSignal(int sig)
	{
		(void)sig;
		gServerRunning = 0;
	}

	void	setupSigpipe()
	{
		if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("signal failed: " + std::string(std::strerror(errno)));
	}

	void	setupShutdownSignals()
	{
		if (signal(SIGINT, handleShutdownSignal) == SIG_ERR
			|| signal(SIGTERM, handleShutdownSignal) == SIG_ERR
			|| signal(SIGQUIT, handleShutdownSignal) == SIG_ERR
			|| signal(SIGHUP, handleShutdownSignal) == SIG_ERR)
			throw std::runtime_error("signal failed: " + std::string(std::strerror(errno)));
	}
}


WebServer::WebServer(const std::string& configFile)
: _numClients(0),
	_listening(true)
{
	// 1: Config (will be in initializer list later)
	(void)configFile; // Ignoring for now
	// 2: Signals
	setupSigpipe();
	setupShutdownSignals();
	// 3: ListenSockets (only one for now)
	ListenSocket*	lsocket = new ListenSocket(_serverConfig, _epoll, *this, "localhost", 8080);
	_ioChannels.push_back(lsocket);
	_lsocks.push_back(lsocket);
}

// Clear open lsockets and csockets (and their associated CGI)
WebServer::~WebServer()
{
	for (std::vector<IOChannel*>::iterator it = _ioChannels.begin(); it != _ioChannels.end(); ++it)
		delete *it;
}

void	WebServer::run()
{
	while (gServerRunning)
	{
		// 1. Wait
		int numSockets = _epoll.wait(1000); // 1s timeout
		// 2. Handle all events
		for (int i = 0; gServerRunning && i < numSockets; ++i)
			reinterpret_cast<IOChannel*>(_epoll[i].data.ptr)->handleEvent(_epoll[i].events);
		// 3. Check connection number
		if (gServerRunning && _listening && _numClients >= _serverConfig.maxConnections)
		{
			_listening = false;
			for (std::vector<ListenSocket*>::iterator it = _lsocks.begin(); it != _lsocks.end(); ++it)
				(*it)->disable();
		}
		else if (gServerRunning && !_listening && _numClients < _serverConfig.maxConnections)
		{
			_listening = true;
			for (std::vector<ListenSocket*>::iterator it = _lsocks.begin(); it != _lsocks.end(); ++it)
				(*it)->enable();
		}
		// 4. Check all session timers
		for (std::vector<IOChannel*>::iterator it = _ioChannels.begin(); gServerRunning && it != _ioChannels.end(); ++it)
			(*it)->checkTimers(std::time(0));
		// 5. Execute pending deletions
		for (std::vector<IOChannel*>::iterator it = _ioChannels.begin(); gServerRunning && it != _ioChannels.end(); ++it)
		{
			if ((*it)->pendingDelete())
			{
				delete *it;
				_ioChannels.erase(it);
			}
		}
	}
}

void	WebServer::addNewClient(IOChannel* client)
{
	_ioChannels.push_back(client);
	_numClients++;
}

void	WebServer::addNewCGI(IOChannel* cgi)
{
	_ioChannels.push_back(cgi);
}