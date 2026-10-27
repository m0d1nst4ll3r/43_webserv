#include "ClientSession.hpp"
#include <ctime>
#include <string>

ClientSession::ClientSession(EpollInstance& epoll, ListenSocket& lsock)
: _socket(epoll, lsock), _websiteConfig(), _serverConfig(), _cgiCall(),
	_request(), _response(), _recvBuffer(), _sendBuffer(), _sendOffset(),
	_state(READING_REQUEST), _lastActivity(std::time(0)), _delete(false)
{

}

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