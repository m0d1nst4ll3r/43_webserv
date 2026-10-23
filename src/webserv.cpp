#include "Color.hpp"
#include "EpollInstance.hpp"
#include "ClientSocket.hpp"
#include "ListenSocket.hpp"
#include <cerrno>
#include <cstring>
#include <csignal>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

volatile sig_atomic_t gServerRunning = 1;

void	handleShutdownSignal(int sig)
{
	(void)sig;
	gServerRunning = 0;
}

void	setupSigpipe()
{
	if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		throw std::runtime_error("signal failed: " + std::string(std::strerror(errno)));
}

void	setupShutdownSignals()
{
	if (signal(SIGINT, handleShutdownSignal) == SIG_ERR
		|| signal(SIGTERM, handleShutdownSignal) == SIG_ERR
		|| signal(SIGQUIT, handleShutdownSignal) == SIG_ERR
		|| signal(SIGHUP, handleShutdownSignal) == SIG_ERR)
		throw std::runtime_error("signal failed: " + std::string(std::strerror(errno)));
}

// Reply with generic header
void	reply(int sockfd, const std::string& msg)
{
	std::ostringstream	oss;
	std::string 		reply;
	reply.append("HTTP/1.1 200 OK\r\n");
	reply.append("Content-Type: text/plain\r\n");
	reply.append("Content-Length: ");
	oss << msg.size();
	reply.append(oss.str());
	reply.append("\r\n\r\n");
	reply.append(msg);
	send(sockfd, reply.c_str(), reply.size(), 0);
}

// No config yet
// 1. SERVER INIT
//   A. Init config (once we have it)
//   B. Init signal
//   C. Init epoll
//   D. Init lsockets (only one without config, arbitrary interface:port)
// 2. SERVER LOOP
//   A. Wait (for now, no timeout)
//   B. Loop over all returned sockets
//     - Identify socket type
//     - Act depending on event and socket type
//        a. 
void	webserv()
{
	setupSigpipe(); // can throw
	setupShutdownSignals(); // can throw
	EpollInstance				epoll; // can throw
	ListenSocket				lsocket(8080, epoll); // can throw
	std::vector<ClientSocket>	clientList;
	while (gServerRunning)
	{
		int numSockets = epoll.wait(-1); // -1 blocks indefinitely, will change
		for (int i = 0; i < numSockets; ++i)
		{
			// Check if ListenSocket or ClientSocket
			// Easy for now since we have 1 ListenSocket
			// Later on, we will have to do a search on our ListenSocket container
			if (epoll[i].data.ptr == &lsocket) // if ListenSocket
			{
				// Check type of event
				if (epoll[i].events & EPOLLHUP || epoll[i].events & EPOLLERR) // HUP or ERR = abort everything for now
					throw std::runtime_error("interface socket closed unexpectedly");
				else if (epoll[i].events & EPOLLIN) // something to accept
				{
					try
					{
						clientList.push_back(ClientSocket(lsocket, epoll));
					}
					catch(const std::exception& e) // Logging for now, might remove or depend on a define
					{
						std::cerr << Color::YELLOW << "Warning" << Color::RESET << ": " << e.what() << '\n';
					}
					
				}
				// EPOLLRDHUP not handled yet
			}
			else // if ClientSocket
			{
				if (epoll[i].events & EPOLLHUP || epoll[i].events & EPOLLERR) // HUP or ERR = close

			}
		}
		// 2. Accept whatever needs accepting and register to epoll (read event) if successful
		// 3. For readable clients, recv (meaning, add to recv buffer)
		//    + If recv buffer contains \r\n\r\n, build request
		//    + If request expects body, continue reading for content-length
		//    + If request is complete (nothing more to recv), register write event to epoll for this socket
		// 4. For writeable clients (meaning we have data to send to them), send (call send and update buffer offset)
		//    + If we have sent everything, unregister write event
	}
}

int	main()
{
	try
	{
		webserv();
	}
	catch(const std::exception& e)
	{
		std::cerr << Color::RED << "Critical Error" << Color::RESET << ": " << e.what() << "\nServer shutting down...\n";
	}
	
	return 0;
}

// Next step:
//
// 1. Make a container for client connections

// Thoughts:
//
// Might be interesting to log everything that happens: config, listen sockets, connections...
//
// Make sure to limit header size, just like body size (body size limit is set in config file)
//
// Remember to limit the number of concurrent client connections. Otherwise, a DDoS attack will crash the server.
// We can also limit the number of connections from a single address or a single address:port to avoid one guy holding
//  all slots hostage (or at least make it harder, they will have to own more addresses).
//
// Remember to honor "Connection: close" - this in a request tells the server to close the connection after sending the response
// Also close after sending an error, as the connection is poisoned.

// Questions:
//
// Q: Do we read additional requests in advance while responding? Do we build a queue of requests for one connection?
// A: No. Gains too little, unnecessary. Build a state machine: READING_REQUEST -> (complete request parsed) -> WRITING_RESPONSE
//     Only start reading the next request after having sent all of the response. Note that the next request might (fully) live
//     inside of the buffer, so check the buffer for a full request first. Also unregister the read event from epoll until we
//     actually want to read again. This makes for a clean design.
//
// Q: If a signal arrives during a recv, or send, or accept, or whatever, it should return an error with errno set as
//  EINTR. However, I can't look at errno, so the best I can do is consider that the call failed, give up the operation,
//  maybe close the connection, and at this point, I will not check my global unless I am the end of my loop, and I won't
//  be at the end of my loop if I am in the middle of my epoll sub-loop that looks at my fd's and reacts to events.
//  So the question is: should I ALSO check my global inside the sub-loop? Probably, right?