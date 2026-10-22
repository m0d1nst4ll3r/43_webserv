#include "ClientSocket.hpp"
#include "ListenSocket.hpp"
#include <stdexcept>
#include <cerrno>
#include <cstring> // strerror
#include <unistd.h> // close
#include <fcntl.h> // fcntl
#include <sys/socket.h>
#include <sys/types.h> // send, recv

// Construct by calling accept() on listen socket
// Also set new socket to O_NONBLOCK (accept4() is not an allowed function)
ClientSocket::ClientSocket(const ListenSocket& lsock)
{
	// This might throw if ListenSocket has no pending connection, which is possible even with polling
	_sockfd = lsock.accept();
	// Set socket to O_NONBLOCK
	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK))
		throw std::runtime_error("fcntl failed: " + std::string(std::strerror(errno)));
}

ClientSocket::~ClientSocket()
{
	close(_sockfd);
}

// Calls a single send()
// Normally, send() will send at least one byte, if the socket was polled
// If send() does not send at least one byte, throw to indicate failure (the connection should be closed)
size_t	ClientSocket::send(const char* buf, size_t size)
{
	ssize_t	ret;
	ret = send(_sockfd, buf, size, 0);
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
	char*			buf[bufsize + 1];
	ssize_t			ret;

	ret = recv(_sockfd, buf, bufsize, 0);
	if (ret < 1)
		throw std::runtime_error("recv failed: " + std::string(std::strerror(errno)));
	buf[ret] = 0;
	return std::string(buf);
}