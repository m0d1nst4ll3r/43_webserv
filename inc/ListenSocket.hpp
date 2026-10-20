#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

class ListenSocket
{
	public:

	// Construction opens new socket, binds it, sets it to listen
	ListenSocket(int port);
	// Destruction closes socket
	~ListenSocket();

	// Attempts accept(), will not block since SOCK_NONBLOCK was set, might still fail and throw
	int accept();

	private:

	int _sockfd;
};

#endif /* LISTENSOCKET_HPP */
