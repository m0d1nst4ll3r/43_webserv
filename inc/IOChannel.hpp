#ifndef IOCHANNEL_HPP
#define IOCHANNEL_HPP

#include "EpollInstance.hpp"
#include "ServerConfig.hpp"
#include <ctime>
#include <stdint.h>

// Interface class for ListenSocket, ClientSession, CGICall
//
// Exposes methods used by WebServer orchestrator, notably to handle epoll events
//  and check if object needs to be deleted.

class IOChannel
{
	public:

	IOChannel(const ServerConfig& serverConfig, EpollInstance& epoll);
	virtual ~IOChannel();

	virtual void	handleEvent(uint32_t events) = 0;
	virtual void	checkTimers(std::time_t now) = 0;
	bool			pendingDelete() const;

	protected:

	const ServerConfig&	_serverConfig;
	//const WebsiteConfig	_websiteConfig; // WIP -> no Config
	EpollInstance&		_epoll;
	bool				_pendingDelete;
};

#endif /* IOCHANNEL_HPP */