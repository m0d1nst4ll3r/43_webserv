#include "IOChannel.hpp"

IOChannel::IOChannel(const ServerConfig& serverConfig, EpollInstance& epoll)
: _serverConfig(serverConfig),
	_epoll(epoll),
	_pendingDelete(false)
{}

IOChannel::~IOChannel()
{}

bool IOChannel::pendingDelete() const
{
	return _pendingDelete;
}