#ifndef CLIENTSESSION_HPP
#define CLIENTSESSION_HPP

#include "ClientSocket.hpp"
#include "ServerConfig.hpp"
#include "EpollInstance.hpp"
#include "CGICall.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ListenSocket.hpp"
#include <ctime>

typedef enum e_state
{
	READING_REQUEST,
	WRITING_CGI,
	READING_CGI,
	SENDING_RESPONSE
} ClientSessionState;

// Incomplete class - WIP
//
// Contains all info pertaining to a client session
//	Objects:
// - The client session socket through which data is received and sent
//		- The socket also contains info about client identity (their address)
//		- This is useful for passing client address to CGI, logging, and rate limiting
// - Website config information: what website does this session belong to (which config to use)
// - Server config information
// - The CGI object representing the CGI call, if any
// - The Request object representing the last received request, if any
// - The Response object representing the response being sent, if any
//
//	Integral types:
// - A recv buffer
// - A send buffer
// - A send offset
// - The session's state
//		- A session is either reading a request, writing to a CGI, reading from a CGI, or sending a response
// - The session's last activity, so server can routinely cull inactive sessions
// - A delete flag indicating whether the ClientSession should be culled by server routine
class ClientSession
{
	public:

	// WIP -> No WebsiteConfig class yet
	//ClientSession(WebsiteConfig& websiteConfig, ServerConfig& serverConfig, EpollInstance& epoll, ListenSocket& lsock);
	ClientSession(ServerConfig& serverConfig, EpollInstance& epoll, ListenSocket& lsock);
	~ClientSession();

	int getFd() const;

	private:

	ClientSocket		_socket;
//	WebsiteConfig*		_websiteConfig;
	ServerConfig*		_serverConfig;
	CGICall*			_cgiCall;
	HTTPRequest*		_request;
	HTTPResponse*		_response;
	std::string			_recvBuffer;
	std::string			_sendBuffer;
	size_t				_sendOffset;
	ClientSessionState	_state;
	std::time_t			_lastActivity;
	bool				_delete;
};

#endif /* CLIENTSESSION_HPP */