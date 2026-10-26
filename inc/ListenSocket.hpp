#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "FileDescriptor.hpp"
#include "EpollInstance.hpp"
#include <string>
#include <sys/socket.h>

// ListenSocket lifecycle:
//
// 1. Instantiate one ListenSocket on startup per interface:port pair
//    Directly set it to NONBLOCK and CLOEXEC and add its fd to epoll
// 2. accept() once after epoll returns EPOLLIN
// 3. Disable epoll events if connection limit is reached, re-enable after slots are freed
// 4. Close at exit

class ListenSocket
{
	public:

	// Construction creates and binds socket, as well as adding it to epoll
	ListenSocket(EpollInstance& epoll, const std::string& interface, int port);
	~ListenSocket();

	// Socket methods
	int accept(struct sockaddr* addr, socklen_t* addrlen);

	// Epoll methods
	void disable();
	void enable();

	// Getter
	int	getFd();

	private:

	EpollInstance*	_epoll;
	FileDescriptor	_sockfd;
};

#endif /* LISTENSOCKET_HPP */
