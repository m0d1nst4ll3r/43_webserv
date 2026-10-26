#include "WebServer.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <csignal>

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
	_listenSockets[lsocket.getFd()] = lsocket;
	std::vector<ClientSocket>	clientList;
}

WebServer::~WebServer()
{
}