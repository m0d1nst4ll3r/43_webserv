#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "FileDescriptor.hpp"
#include "EpollInstance.hpp"

// Expected ListenSocket usage in WebServ:
//
// 1. Instantiate one ListenSocket on startup per interface:port pair
//    Directly set it to NONBLOCK and add its fd to epoll
// 2. accept() once after epoll returns EPOLLIN
// 3. Disable epoll if connection limit is reached, re-enable after slots are freed
// 4. Close at exit

class ListenSocket
{
	public:

	// Construction opens new socket, binds it, sets it to listen, adds it to epoll
	// Throws if any socket, bind, setsockopt, listen call fails
	ListenSocket(int port, EpollInstance& epoll);
	// Destruction closes socket
	~ListenSocket();

	// Attempts accept(), will not block since SOCK_NONBLOCK was set, might still fail and throw
	int accept();

	// Unregister EPOLLIN from epoll, can throw
	void disable();
	// Register EPOLLIN to epoll, can throw
	void enable();

	private:

	EpollInstance* _epoll;
	FileDescriptor _sockfd;
};

#endif /* LISTENSOCKET_HPP */
