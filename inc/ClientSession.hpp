#ifndef CLIENTSESSION_HPP
#define CLIENTSESSION_HPP

#include <ctime>
#include "ClientSocket.hpp"

typedef enum e_state
{
	READING_REQUEST,
	WRITING_CGI,
	READING_CGI,
	SENDING_RESPONSE
} ClientSessionState;

// Contains all info pertaining to a client session
class ClientSession
{
	public:

	ClientSession();
	~ClientSession();

	int getFd() const;

	private:

	ClientSocket		_socket;
	std::time_t			_lastActivity;
	ClientSessionState	_state;
};

#endif /* CLIENTSESSION_HPP */