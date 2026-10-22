#include "EpollInstance.hpp"
#include <sys/epoll.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <string>
#include <unistd.h>

EpollInstance::EpollInstance()
{
	_fd = epoll_create1(0);
	if (_fd < 0)
		throw std::runtime_error("epoll_create failed: " + std::string(std::strerror(errno)));
}

EpollInstance::~EpollInstance()
{
	close(_fd);
}

void EpollInstance::addRecv(int fd)
{
	struct epoll_event	ev = {};
	ev.events = EPOLLIN;
	epoll_ctl(_fd, EPOLL_CTL_ADD, fd, EPOLLIN);
}

void EpollInstance::rmRecv(int fd)
{

}

void EpollInstance::addSend(int fd)
{

}

void EpollInstance::rmSend(int fd)
{

}