#ifndef EPOLLINSTANCE_HPP
#define EPOLLINSTANCE_HPP

#include "FileDescriptor.hpp"
#include <sys/epoll.h>
#include <vector>

// Expected epoll usage in WebServ:
//
// 1. Create epoll instance at startup
// 2. Always add sockets with EPOLLIN on creation
// 3. Remove EPOLLIN from listen sockets on connection limit, re-add when slots free up
// 4. Switch EPOLLIN to EPOLLOUT when client socket switches from READING state to SENDING
// 5. Switch EPOLLOUT to EPOLLIN when client socket switches from SENDING state to READING
// 6. Close epoll instance at exit
//
// Additionally, wait on every loop iteration and act on returned events:
//  - EPOLLIN	accept() on listen socket or recv() on client socket
//  - EPOLLOUT	send() on client socket
//  - EPOLLHUP, EPOLLERR	close the socket

class EpollInstance
{
	public:

	// Create epoll instance, can throw
	EpollInstance();
	// Close fd
	~EpollInstance();

	// Register fd and events to epoll, can throw
	// Throws if fd is already registered (use mod)
	void	add(int fd, void *ptr, uint32_t events);
	// Register fd and events to epoll, can throw
	// Throws if fd is not registered (use add)
	void	mod(int fd, void *ptr, uint32_t events);
	// Unregister fd, does not throw (errors are ignored)
	void	del(int fd);
	// Run epoll_wait using own events array, can throw
	int		wait(int timeout_ms);

	// Does not bound check
	struct epoll_event operator[](int i) const;

	private:

	FileDescriptor					_fd;
	std::vector<struct epoll_event>	_events;
};

#endif /* EPOLLINSTANCE_HPP */