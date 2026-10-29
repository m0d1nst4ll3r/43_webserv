#include "ClientSession.hpp"
#include <ctime>
#include <string>
#include <iostream>

// WIP - No WebsiteConfig class yet
ClientSession::ClientSession(const ServerConfig& serverConfig, EpollInstance& epoll, WebServer& webServer, ListenSocket& lsock)
: IOChannel(serverConfig, epoll),
	_webServer(webServer),
	_socket(epoll, *this, lsock),
	_cgiCall(0),
	_request(0),
	_response(0),
	_recvBuffer(),
	_sendBuffer(),
	_sendOffset(0),
	_state(READING_REQUEST),
	_lastActivity(std::time(0))
{}

ClientSession::~ClientSession()
{
	_webServer.removeClient();
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

// Logic:
// 0. Do recv
// 1. recv fails -> close connection, just set pendingDelete to true
// 2. recv succeeds
//		a. add to recv buffer
//		b. check request end
//		c. check size (only request if request found, all buffer if not)
//			- size too big? construct error response, go to SENDING_RESPONSE
//		d. if request ended and size not too big, go to 3, else do nothing
// 3. construct request object
//		- this might fail because of bad request -> construct error response, go to SENDING_RESPONSE
// 4. check if request asks for body, and how (content-length, or chunk ecoding)
// 5. keep reading for body, but now either wait for reading content-length or chunk encoding is not too big
// 6. once body is read fully, construct response (or do CGI call)
void	ClientSession::_handleEpollin()
{
	// For now, logic is not implemented, just recv until \r\n\r\n
	try
	{
		_recvBuffer.append(_socket.recv()); // No size check yet
		if (_recvBuffer.find("\r\n\r\n") != std::string::npos) // Found end
		{ // Just construct fake response and go to SENDING state right away
			_sendBuffer.append("HTTP/1.1 200 OK\r\n");
			_sendBuffer.append("Connection: close\r\n"); // closing connection right away for simplicity
			_sendBuffer.append("Content-Type: text/plain\r\n");
			_sendBuffer.append("Content-Length: 12");
			_sendBuffer.append("\r\n\r\n");
			_sendBuffer.append("Hello world!");
			_socket.setSend();
			_state = SENDING_RESPONSE;
		}
	}
	catch(const std::exception& e)
	{ // Just close connection for now, and log
		std::cout << "ClientSession EPOLLIN: " << e.what() << "\n";
		_pendingDelete = true;
	}
}

// WIP Note: should change strings to vector<char> to handle null bytes (what if we receive a binary with POST?)
void	ClientSession::_handleEpollout()
{
	try
	{
		_sendOffset += _socket.send(_sendBuffer.c_str() + _sendOffset, _sendBuffer.size() - _sendOffset);
		if (_sendOffset == _sendBuffer.size()) // all sent, close (for now)
			_pendingDelete = true;
	}
	catch(const std::exception& e)
	{ // Just close connection for now, and log
		std::cout << "ClientSession EPOLLOUT: " << e.what() << "\n";
		_pendingDelete = true;
	}
	
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
		_handleEpollin();
	else if (events & EPOLLOUT)
		_handleEpollout();
	else
	{
		std::cout << "Connection closed\n"; // Logging for debug, for now
		_pendingDelete = true;
	}
}

void	ClientSession::checkTimers(std::time_t now)
{
	// WIP
	(void)now;
}