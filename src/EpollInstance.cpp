#include "EpollInstance.hpp"
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h> // epoll_create1, epoll_wait, epoll_ctl
#include <cstdint> // uint32_t
#include <string>
#include <vector>

namespace
{
	int createEpoll()
	{
		int fd = epoll_create1(0);
		if (fd < 0)
			throw std::runtime_error("epoll_create failed: " + std::string(std::strerror(errno)));
		return fd;
	}
}

// Create instance
EpollInstance::EpollInstance() : _events(64), _epollfd(createEpoll()) {}

// Close instance
EpollInstance::~EpollInstance() {}

// Add new fd
void	EpollInstance::add(int fd, void *ptr, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(_epollfd.fd, EPOLL_CTL_ADD, fd, &ev) < 0)
		throw std::runtime_error("epoll_ctl (add) failed: " + std::string(std::strerror(errno)));
}

// Mod existing fd
void	EpollInstance::mod(int fd, void *ptr, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(_epollfd.fd, EPOLL_CTL_MOD, fd, &ev) < 0)
		throw std::runtime_error("epoll_ctl (mod) failed: " + std::string(std::strerror(errno)));
}

// Remove existing fd (ignores errors)
void	EpollInstance::del(int fd)
{
	epoll_ctl(_epollfd.fd, EPOLL_CTL_DEL, fd, 0);
}

// Wait for events
int		EpollInstance::wait(int timeout_ms)
{
	int ret;
	ret = epoll_wait(_epollfd.fd, &_events[0], 64, timeout_ms);
	if (ret < 0)
		throw std::runtime_error("epoll_wait failed: " + std::string(std::strerror(errno)));
	return ret;
}

struct epoll_event EpollInstance::operator[](int i) const
{
	return _events[i];
}