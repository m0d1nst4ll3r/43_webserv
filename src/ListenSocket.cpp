#include "ListenSocket.hpp"
#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring> // strerror
#include <arpa/inet.h> // htons htonl
#include <sys/socket.h>
#include <sys/types.h> // socket, bind, listen, setsockopt
#include <unistd.h> // close

ListenSocket::ListenSocket(int port)
{
	// Open new socket
	_sockfd = socket(AF_INET, SOCK_STREAM /*| SOCK_NONBLOCK*/, 0); // Set NONBLOCK so socket is non-blocking
	if (_sockfd < 0)
		throw std::runtime_error("socket failed: " + std::string(std::strerror(errno)));
	// Set SO_REUSEADDR to avoid "Address already in use" errors
	const int enable = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("setsockopt failed: " + std::string(std::strerror(errno)));
	}
	// Bind socket
	struct sockaddr_in sa = {}; // Zero fill
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port); // htons and htonl convert to network byte order (big endian)
	sa.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY is technically 0, but keeping htonl for clarity
	if (bind(_sockfd, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("bind failed: " + std::string(std::strerror(errno)));
	}
	// Set socket to listen
	if (listen(_sockfd, SOMAXCONN) < 0)
	{
		close(_sockfd);
		throw std::runtime_error("listen failed: " + std::string(std::strerror(errno)));
	}
}

ListenSocket::~ListenSocket()
{
	close(_sockfd);
}

// Make sure socket is polled before calling, or call will throw with EAGAIN or EWOULDBLOCK
// Note that this will still throw if connection is lost between poll and call
int ListenSocket::accept()
{
	int clientfd;
	clientfd = ::accept(_sockfd, 0, 0);
	if (clientfd < 0)
		throw std::runtime_error("accept failed: " + std::string(std::strerror(errno)));
	return clientfd;
}
