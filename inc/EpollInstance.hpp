#ifndef EPOLLINSTANCE_HPP
#define EPOLLINSTANCE_HPP

#include <sys/epoll.h>

class EpollInstance
{
	public:

	// Create epoll instance, can throw
	EpollInstance();
	// Close _fd
	~EpollInstance();

	// Register read event for fd
	// Unregister read event for fd
	// Register write event for fd
	// Unregister write event for fd

	private:

	int	_fd;
}

#endif /* EPOLLINSTANCE_HPP */