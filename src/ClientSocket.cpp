#include "FileDescriptor.hpp"
#include "ClientSocket.hpp"
#include "ListenSocket.hpp"
#include "EpollInstance.hpp"
#include <stdexcept>
#include <cerrno>
#include <cstring> // strerror
#include <unistd.h> // close
#include <fcntl.h> // fcntl
#include <sys/socket.h>
#include <sys/types.h> // send, recv
#include <string>

// Construct by calling accept() on listen socket
// Also set new socket to O_NONBLOCK (accept4() is not an allowed function)
// This might throw if ListenSocket has no pending connection, which is possible even with polling
ClientSocket::ClientSocket(ListenSocket& lsock, EpollInstance& epoll) : _epoll(&epoll), _sockfd(lsock.accept()), _clientAddr(), _addrLen(sizeof(_clientAddr))
{
	// Set socket to O_NONBLOCK
	if (fcntl(_sockfd.fd, F_SETFL, O_NONBLOCK))
		throw std::runtime_error("fcntl failed: " + std::string(std::strerror(errno)));
	// Add socket to epoll (can throw)
	_epoll->add(_sockfd.fd, this, EPOLLIN);
}

ClientSocket::~ClientSocket()
{
	_epoll->del(_sockfd.fd);
}

// Calls a single send()
// Normally, send() will send at least one byte, if the socket was polled
// If send() does not send at least one byte, throw to indicate failure (the connection should be closed)
size_t	ClientSocket::send(const char* buf, size_t size)
{
	ssize_t	ret;
	ret = ::send(_sockfd.fd, buf, size, 0);
	if (ret < 1)
		throw std::runtime_error("send failed: " + std::string(std::strerror(errno)));
	return ret;
}

// Calls a single recv()
// Normally, recv() will read at least one byte, if the socket was polled
// If recv() does not read at least one byte, throw to indicate failure (the connection should be closed)
std::string	ClientSocket::recv()
{
	const size_t	bufsize = 16383; // 16 kB
	char			buf[bufsize + 1];
	ssize_t			ret;

	ret = ::recv(_sockfd.fd, buf, bufsize, 0);
	if (ret < 1)
		throw std::runtime_error("recv failed: " + std::string(std::strerror(errno)));
	buf[ret] = 0;
	return std::string(buf);
}

void	ClientSocket::setRecv()
{
	_epoll->mod(_sockfd.fd, this, EPOLLIN);
}

void	ClientSocket::setSend()
{
	_epoll->mod(_sockfd.fd, this, EPOLLOUT);
}