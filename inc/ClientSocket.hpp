#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "FileDescriptor.hpp"
#include "ListenSocket.hpp"
#include "EpollInstance.hpp"
#include "IOChannel.hpp"
#include <string>

class ClientSession;

// ClientSocket lifecycle:
//
// 1. Instantiate new ClientSocket after epoll returned EPOLLIN on a ListenSocket
//    Directly set it to NONBLOCK and CLOEXEC and add its fd to epoll
// 2. recv() once after epoll returns EPOLLIN
// 3. send() once after epoll returns EPOLLOUT
// 4. Change registered event to EPOLLIN during READING, and EPOLLOUT during SENDING
// 5. Close socket once epoll returns EPOLLHUP/EPOLLERR, or client sends Connection: close

class ClientSocket
{
	public:

	// Constructed from a ListenSocket accept()
	ClientSocket(EpollInstance& epoll, ClientSession& clientSession, ListenSocket& lsock);
	~ClientSocket();

	// IO methods
	size_t		send(const char* buf, size_t size);
	std::string	recv();

	// Epoll methods
	void	setRecv();
	void	setSend();

	// Getter
	int getFd() const;

	private:

	EpollInstance*			_epoll;
	ClientSession&			_clientSession;
	struct sockaddr_storage	_clientAddr; // Address of socket peer
	socklen_t				_addrLen;
	FileDescriptor			_sockfd;
};

#endif /* CLIENTSOCKET_HPP */
