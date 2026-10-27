#ifndef EPOLLINSTANCE_HPP
#define EPOLLINSTANCE_HPP

#include "FileDescriptor.hpp"
#include <sys/epoll.h>
#include <vector>

// EpollInstance lifecycle:
//
// 1. Create epoll instance at startup, allowing epoll event registration
// 2. Wait will be called in a loop by web server
// 3. Close epoll instance at exit

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