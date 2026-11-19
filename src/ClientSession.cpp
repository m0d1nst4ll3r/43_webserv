#include "ClientSession.hpp"
#include "Config.hpp"
#include "EpollInstance.hpp"
#include "WebServer.hpp"
#include "ListenSocket.hpp"
#include "IOChannel.hpp"
#include "ClientSocket.hpp"
#include "CGICall.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include <ctime>
#include <string>
#include <iostream>
#include <limits>
#include <sstream>

// WIP - No WebsiteConfig class yet
ClientSession::ClientSession(const ServerConfig& serverConfig, const Config& websiteConfig, EpollInstance& epoll, WebServer& webServer, ListenSocket& lsock)
: IOChannel(serverConfig, websiteConfig, epoll),
	_bodyBuffer(),
	_webServer(webServer),
	_socket(epoll, *this, lsock),
	_cgiCall(0),
	_request(0),
	_response(0),
	_recvBuffer(),
	_sendBuffer(),
	_sendOffset(0),
	_state(READING_REQUEST),
	_lastActivity(std::time(0)),
	_expectedBodyLength(0)
{
	_webServer.addNewClient(this);
}

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
	if (_state == READING_BODY)
	{
		ClientSocket::ReceiveResult result = _socket.recv();

		if (result.status == ClientSocket::RECEIVE_DATA)
		{
			_bodyBuffer.append(result.data);
			_lastActivity = std::time(0);

			if (_bodyBuffer.size() == _expectedBodyLength)
			{
				_startResponse200();
				return;
			}
			else if (_bodyBuffer.size() > _expectedBodyLength)
			{
				_startErrorResponse(
					400,
					"Bad Request",
					"Invalid request body.\n"
				);
			}
		}
		else if (result.status == ClientSocket::RECEIVE_CLOSED)
		{
			_pendingDelete = true;
		}
		else if (result.status == ClientSocket::RECEIVE_ERROR)
		{
			_pendingDelete = true;
		}

		return;
	}

	try
	{
		ClientSocket::ReceiveResult result = _socket.recv();
		std::string	headers;

		if (result.status == ClientSocket::RECEIVE_DATA)
		{
			_recvBuffer.append(result.data);
			_lastActivity = std::time(0);
			std::size_t separator = _recvBuffer.find("\r\n\r\n");
			if (separator == std::string::npos)
			{
				if (_recvBuffer.size() > _serverConfig.requestHeaderMaxLength)
				{
					_startErrorResponse(431, "Request Header Fields Too Large", "Request headers too large.\n");
					return;
				}
				return;
			}
			if (separator + 4 > _serverConfig.requestHeaderMaxLength)
			{
				_startErrorResponse(431, "Request Header Fields Too Large", "Request headers too large.\n");
				return;
			}
			headers = _recvBuffer.substr(0, separator);
			_bodyBuffer = _recvBuffer.substr(separator + 4);
			_recvBuffer.clear();

			try
			{
				if (_request)
				{	
					delete _request;
					_request = 0;
				}

				_request = new HTTPRequest(headers, _serverConfig.legalMethods, _serverConfig.legalProtocols);

				if (_request->hasHeader("transfer-encoding"))
				{
					_startErrorResponse(
						501,
						"Not Implemented",
						"Chunked transfer encoding is not supported.\n"
					);
					return;
				}
				else if (_request->hasHeader("content-length"))
				{
					const std::string& value =
						_request->getHeader("content-length");

					if (value.empty())
						throw std::runtime_error("Error: invalid Content-Length value.");

					std::size_t convertedValue = 0;

					for (std::size_t i = 0; i < value.size(); ++i)
					{
						if (value[i] < '0' || value[i] > '9')
							throw std::runtime_error("Error: invalid Content-Length value.");

						std::size_t digit = value[i] - '0';

						if (convertedValue >
							(std::numeric_limits<std::size_t>::max() - digit) / 10)
							throw std::runtime_error("Error: invalid Content-Length value.");

						convertedValue = convertedValue * 10 + digit;
					}

					if (convertedValue > _serverConfig.requestBodyMaxLength
						|| convertedValue > _websiteConfig.maxBodySize)
						throw std::runtime_error("Error: Content-Length value too big.");

					_expectedBodyLength = convertedValue;

					if (_bodyBuffer.size() < _expectedBodyLength)
						_state = READING_BODY;
					else if (_bodyBuffer.size() == _expectedBodyLength)
						_startResponse200();
					else
						_startErrorResponse(400, "Bad Request", "Invalid request body.\n");
				}
				else
					_startResponse200();
			}
			catch (const std::exception&)
			{
				_startErrorResponse(400, "Bad Request", "Malformed request.\n");
				return;
			}
		}	
		else if (result.status == ClientSocket::RECEIVE_CLOSED)
			_pendingDelete = true;
		else if (result.status == ClientSocket::RECEIVE_ERROR)
			_pendingDelete = true;
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
	{
		if (_state == READING_REQUEST)
			_handleEpollin();
		else if (_state == READING_BODY)
			_handleEpollin();
	}
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

void	ClientSession::_startErrorResponse(int code, std::string reason, std::string body)
{
	if (_response)
	{
		delete _response;
		_response = 0;
	}

	_response = new HTTPResponse(code);
	_response->setStatusCode(code);
	_response->setReasonPhrase(reason);
	_response->setHeader("Content-Type", "text/plain");
	_response->setBody(body);

	_sendBuffer = _response->toString();
	_sendOffset = 0;
	_state = SENDING_RESPONSE;
	_socket.setSend();
}

void ClientSession::_startResponse200()
{
    if (_response)
    {
        delete _response;
        _response = 0;
    }

    _response = new HTTPResponse(200);
    _response->setStatusCode(200);
    _response->setReasonPhrase("OK");
    _response->setHeader("Content-Type", "text/plain");
    _response->setBody("Hello, World!\n");

    _sendBuffer = _response->toString();
    _sendOffset = 0;
    _state = SENDING_RESPONSE;
    _socket.setSend();
}