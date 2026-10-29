#include "IOChannel.hpp"
#include "Config.hpp"

IOChannel::IOChannel(const ServerConfig& serverConfig, const Config& websiteConfig, EpollInstance& epoll)
: _serverConfig(serverConfig),
	_websiteConfig(websiteConfig),
	_epoll(epoll),
	_pendingDelete(false)
{}

IOChannel::~IOChannel()
{}

bool IOChannel::pendingDelete() const
{
	return _pendingDelete;
}