#ifndef EPOLLINSTANCE_HPP
#define EPOLLINSTANCE_HPP

#include <sys/epoll.h>

class EpollInstance
{
	public:

	// Create epoll instance, can throw
	EpollInstance();
	// Close fd
	~EpollInstance();

	// Register recv event for fd
	void addRecv(int fd);
	// Unregister recv event for fd
	void rmRecv(int fd);
	// Register send event for fd
	void addSend(int fd);
	// Unregister send event for fd
	void rmSend(int fd);

	private:

	int	_fd;
}

#endif /* EPOLLINSTANCE_HPP */