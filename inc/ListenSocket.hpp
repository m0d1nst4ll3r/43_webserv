#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "ServerConfig.hpp"
#include "WebServer.hpp"
#include "IOChannel.hpp"
#include "FileDescriptor.hpp"
#include "EpollInstance.hpp"
#include <string>
#include <sys/socket.h>
#include <ctime>
#include <cstdint>

// ListenSocket lifecycle:
//
// 1. Instantiate one ListenSocket on startup per interface:port pair
//    Directly set it to NONBLOCK and CLOEXEC and add its fd to epoll
// 2. accept() once after epoll returns EPOLLIN
// 3. Disable epoll events if connection limit is reached, re-enable after slots are freed
// 4. Close at exit

class ListenSocket : public IOChannel
{
	public:

	ListenSocket(const ServerConfig& serverConfig, EpollInstance& epoll, WebServer& webServer, const std::string& interface, int port);
	~ListenSocket();

	// Socket methods
	int accept(struct sockaddr* addr, socklen_t* addrlen);

	// Epoll methods
	void disable();
	void enable();

	// Getter
	int	getFd();

	// IOChannel interface
	virtual void	handleEvent(uint32_t events);
	virtual void	checkTimers(std::time_t now);

	private:

	WebServer&		_webServer;
	std::string		_interface;
	int				_port;
	FileDescriptor	_sockfd;
};

#endif /* LISTENSOCKET_HPP */