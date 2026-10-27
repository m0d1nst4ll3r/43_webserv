#include "WebServer.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <csignal>
#include <stdexcept>

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


WebServer::WebServer(const std::string& configFile) : _epoll()
{
	// 1: CONFIG (will be in initializer list later)
	(void)configFile; // Ignoring for now
	// 2: SIGNALS
	setupSigpipe();
	setupShutdownSignals();
	// 3: LSOCKETS
	ListenSocket*	lsocket = new ListenSocket(8080, _epoll);
	_listenSockets[lsocket->getFd()] = lsocket;
	std::vector<ClientSocket>	clientList;
}

WebServer::~WebServer()
{
}

// No config yet
// 1. SERVER INIT
//   A. Init config (once we have it)
//   B. Init signal
//   C. Init epoll
//   D. Init lsockets (only one without config, arbitrary interface:port)
// 2. SERVER LOOP
//   A. Wait (for now, no timeout)
//   B. Loop over all returned sockets
//     - Identify socket type
//     - Act depending on event and socket type
//        a. 
void	WebServer::run()
{
	while (gServerRunning)
	{
		int numSockets = _epoll.wait(-1); // -1 blocks indefinitely, will change
		for (int i = 0; i < numSockets; ++i)
		{
			// Check if ListenSocket or ClientSocket
			// Easy for now since we have 1 ListenSocket
			// Later on, we will have to do a search on our ListenSocket container
			if (_epoll[i].data.ptr == &lsocket) // if ListenSocket
			{
				// Check type of event
				if (_epoll[i].events & EPOLLHUP || epoll[i].events & EPOLLERR) // HUP or ERR = abort everything for now
					throw std::runtime_error("interface socket closed unexpectedly");
				else if (epoll[i].events & EPOLLIN) // something to accept
				{
					try
					{
						clientList.push_back(ClientSocket(lsocket, epoll));
					}
					catch(const std::exception& e) // Logging for now, might remove or depend on a define
					{
						std::cerr << Color::YELLOW << "Warning" << Color::RESET << ": " << e.what() << '\n';
					}
					
				}
				// EPOLLRDHUP not handled yet
			}
			else // if ClientSocket
			{
				if (_epoll[i].events & EPOLLHUP || epoll[i].events & EPOLLERR) // HUP or ERR = close

			}
		}
	}
}