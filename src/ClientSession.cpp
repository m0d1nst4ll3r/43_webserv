#include "ClientSession.hpp"
#include <ctime>
#include <string>

// WIP - No WebsiteConfig class yet
ClientSession::ClientSession(const ServerConfig& serverConfig, EpollInstance& epoll, WebServer& webServer, ListenSocket& lsock)
: IOChannel(serverConfig, epoll),
	_webServer(webServer),
	_socket(epoll, *this, lsock),
	_state(READING_REQUEST),
	_lastActivity(std::time(0))
{}

ClientSession::~ClientSession()
{
	if (_cgiCall)
		delete _cgiCall;
	if (_request)
		delete _request;
	if (_response)
		delete _response;
}

int	ClientSession::getFd() const
{
	return _socket.getFd();
}

// ListenSocket event handler:
//
// - EPOLLERR		Connection close -> set pendingDelete
// - EPOLLHUP		-
// - EPOLLRDHUP		/ (WIP)
// - EPOLLIN		/
// - EPOLLOUT		/
void	ClientSession::handleEvent(uint32_t events)
{
	if (events & EPOLLIN)
		;
	else if (events & EPOLLOUT)
		;
	else
		_pendingDelete = true;
}

void	ClientSession::checkTimers(std::time_t now)
{
	// WIP
	(void)now;
}