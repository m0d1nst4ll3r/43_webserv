#include "FileDescriptor.hpp"
#include "ListenSocket.hpp"
#include "EpollInstance.hpp"
#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring> // strerror
#include <arpa/inet.h> // htons htonl
#include <sys/socket.h>
#include <sys/types.h> // socket, bind, listen, setsockopt
#include <unistd.h> // close

namespace
{
	
	int createSocket()
	{
		int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // Set NONBLOCK so socket is non-blocking
		if (sockfd < 0)
			throw std::runtime_error("socket failed: " + std::string(std::strerror(errno)));
		return sockfd;
	}

	void setSocketReuse(int fd)
	{
		const int enable = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
			throw std::runtime_error("setsockopt failed: " + std::string(std::strerror(errno)));
	}

	void bindSocket(int fd, int port)
	{
		struct sockaddr_in sa = {}; // Zero fill
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port); // htons and htonl convert to network byte order (big endian)
		sa.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY is technically 0, but keeping htonl for clarity
		if (bind(fd, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0)
			throw std::runtime_error("bind failed: " + std::string(std::strerror(errno)));
	}

	void setSocketListen(int fd)
	{
		if (listen(fd, SOMAXCONN) < 0)
			throw std::runtime_error("listen failed: " + std::string(std::strerror(errno)));
	}
}

ListenSocket::ListenSocket(int port, EpollInstance& epoll) : _epoll(&epoll), _sockfd(createSocket())
{
	setSocketReuse(_sockfd.fd()); // Set SO_REUSEADDR to avoid "Address already in use" errors
	bindSocket(_sockfd.fd(), port);
	setSocketListen(_sockfd.fd());
	_epoll->add(_sockfd.fd(), this, EPOLLIN); // Add socket to epoll, can throw
}

ListenSocket::~ListenSocket()
{
	_epoll->del(_sockfd.fd());
}

// Make sure socket is polled before calling, or call will throw with EAGAIN or EWOULDBLOCK
// Note that this will still throw if connection is lost between poll and call
int ListenSocket::accept()
{
	int clientfd;
	clientfd = ::accept(_sockfd.fd(), 0, 0);
	if (clientfd < 0)
		throw std::runtime_error("accept failed: " + std::string(std::strerror(errno)));
	return clientfd;
}

void ListenSocket::disable()
{
	_epoll->mod(_sockfd.fd(), this, 0);
}

void ListenSocket::enable()
{
	_epoll->mod(_sockfd.fd(), this, EPOLLIN);
}