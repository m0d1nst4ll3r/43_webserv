#include "ListenSocket.hpp"
#include "ServerConfig.hpp"
#include "IOChannel.hpp"
#include "FileDescriptor.hpp"
#include "EpollInstance.hpp"
#include "ClientSession.hpp"
#include "Color.hpp"
#include <stdexcept>
#include <cerrno>
#include <cstring> // strerror
#include <arpa/inet.h> // htons htonl
#include <sys/socket.h>
#include <sys/types.h> // socket, bind, listen, setsockopt
#include <netdb.h> // getaddrinfo, freeaddrinfo, gai_strerror
#include <unistd.h> // close
#include <stdint.h> // uint32_t
#include <sstream> // convert int port to string
#include <string>
#include <iostream>
#include <ctime>

namespace
{
	std::string	toString(int n)
	{
		std::stringstream ss;
		ss << n;
		return ss.str();
	}

	// Create socket and bind directly -- has to be done together because of IPv6
	int createSocket(int port, const std::string& interfaceStr)
	{
		int					ret;
		struct addrinfo*	res = NULL;

		// 1. Zero fill and configure hints
		struct addrinfo		hints = {};
		hints.ai_family = AF_UNSPEC;		// IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM;	// TCP stream
		hints.ai_protocol = IPPROTO_TCP;	// TCP protocol
		hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // Intended for binding & passing port as number

		// 2. Resolve addresses
		std::string	portStr = toString(port);
		ret = getaddrinfo(interfaceStr.c_str(), portStr.c_str(), &hints, &res);
		if (ret)
			throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(ret)));

		// 3. Loop through list, attempting to bind
		int					fd;
		struct addrinfo*	cur = res;
		while (cur)
		{
			fd = socket(cur->ai_family, cur->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC, cur->ai_protocol);
			if (fd >= 0)
			{
				if (bind(fd, cur->ai_addr, cur->ai_addrlen) == 0)
					break; // Success
				close(fd); // Failure
			}
			cur = cur->ai_next;
		}

		// Free res linked list
		freeaddrinfo(res);

		if (!cur) // No bindable address found, throw
			throw std::runtime_error("cannot bind socket to '" + (interfaceStr.empty() ? "0.0.0.0" : interfaceStr) + ":" + portStr);

		return fd;
	}

	void setSocketReuse(int fd)
	{
		const int enable = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
			throw std::runtime_error("setsockopt failed: " + std::string(std::strerror(errno)));
	}

	void setSocketListen(int fd)
	{
		if (listen(fd, SOMAXCONN) < 0)
			throw std::runtime_error("listen failed: " + std::string(std::strerror(errno)));
	}
}

//ListenSocket::ListenSocket(const ServerConfig& serverConfig, const WebsiteConfig& websiteConfig, EpollInstance& epoll, const std::string& interface, int port);
ListenSocket::ListenSocket(const ServerConfig& serverConfig, EpollInstance& epoll, WebServer& WebServer, const std::string& interface, int port)
: IOChannel(serverConfig, epoll),
	_webServer(WebServer),
	_interface(interface.empty() ? "0.0.0.0" : interface),
	_port(port),
	_sockfd(createSocket(port, interface))
{
	setSocketReuse(_sockfd.fd); // Set SO_REUSEADDR to avoid "Address already in use" errors
	setSocketListen(_sockfd.fd);
	_epoll.add(_sockfd.fd, this, EPOLLIN); // Add socket to epoll, can throw
}

ListenSocket::~ListenSocket()
{
	_epoll.del(_sockfd.fd);
}

// Make sure socket is polled before calling, or call will throw with EAGAIN or EWOULDBLOCK
// Note that this will still throw if connection is lost between poll and call
int ListenSocket::accept(struct sockaddr* addr, socklen_t* addrlen)
{
	int clientfd;
	clientfd = ::accept(_sockfd.fd, addr, addrlen);
	if (clientfd < 0)
		throw std::runtime_error("accept failed: " + std::string(std::strerror(errno)));
	return clientfd;
}

void ListenSocket::disable()
{
	_epoll.mod(_sockfd.fd, this, 0);
}

void ListenSocket::enable()
{
	_epoll.mod(_sockfd.fd, this, EPOLLIN);
}

int	ListenSocket::getFd()
{
	return _sockfd.fd;
}

// ListenSocket event handler:
//
// - EPOLLERR		Throw with descriptive error, server should abort
// - EPOLLHUP		-
// - EPOLLRDHUP		-
// - EPOLLIN		Create new ClientSession
void	ListenSocket::handleEvent(uint32_t events)
{
	if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
	{
		std::stringstream ss;
		ss << _port;
		throw std::runtime_error("unexpected error on " + _interface + ":" + ss.str() + " socket");
	}
	if (events & EPOLLIN)
	{
		try
		{
			ClientSession* client = new ClientSession(_serverConfig, _epoll, _webServer, *this);
			_webServer.addNewClient(client);
		}
		catch(const std::exception& e)
		{ // Log error, but do nothing else
			std::cerr << Color::YELLOW << "Warning" << Color::RESET << ": ClientSession init failed: " << e.what() << "\n";
		}
	}
}

// Nothing to do - listen sockets are persistent
void		ListenSocket::checkTimers(std::time_t now)
{
	(void)now;
}