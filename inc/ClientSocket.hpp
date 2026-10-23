#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "ListenSocket.hpp"

class ClientSocket
{
	public:

	// Constructed from a ListenSocket (can throw)
	ClientSocket(const ListenSocket& lsock);
	// Destruction closes socket
	~ClientSocket();

	// Attempt to send or recv data through socket (can throw)
	// Sending or receiving only calls send & recv ONCE per call
	// If send or recv fail (0 or -1), they throw
	size_t		send(const char* buf, size_t size);
	std::string	recv();
	// Sending returns how many bytes were sent
	// Receiving returns received bytes in string form

	private:

	int _sockfd;
};

#endif /* CLIENTSOCKET_HPP */
