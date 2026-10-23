#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "FileDescriptor.hpp"
#include "ListenSocket.hpp"
#include "EpollInstance.hpp"
#include <string>

// Expected ClientSocket usage in WebServ:
//
// 1. Instantiate new ClientSocket after epoll returned EPOLLIN on a ListenSocket
//    Directly set it to NONBLOCK and add its fd to epoll
// 2. recv() once after epoll returns EPOLLIN
// 3. send() once after epoll returns EPOLLOUT
// 4. Change registered event to EPOLLIN during READING, and EPOLLOUT during SENDING
// 5. Close socket once epoll returns EPOLLHUP/EPOLLERR, or client sends Connection: close

class ClientSocket
{
	public:

	// Constructed from a ListenSocket (can throw)
	ClientSocket(ListenSocket& lsock, EpollInstance& epoll);
	// Destruction closes socket
	~ClientSocket();

	// Attempt to send or recv data through socket (can throw)
	// Sending or receiving only calls send & recv ONCE per call
	// If send or recv fail (0 or -1), they throw
	size_t		send(const char* buf, size_t size);
	std::string	recv();
	// Sending returns how many bytes were sent
	// Receiving returns received bytes in string form

	// Change registered event to EPOLLIN (can throw)
	void	setRecv();
	// Change registered event to EPOLLOUT (can throw)
	void	setSend();

	private:

	EpollInstance* _epoll;
	FileDescriptor _sockfd;
};

#endif /* CLIENTSOCKET_HPP */
