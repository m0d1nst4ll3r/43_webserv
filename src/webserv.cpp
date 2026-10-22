#include "ListenSocket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

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

void	webserv()
{
	// No config yet, no signal setup, etc...
	// Init epoll
	int				epoll_fd = epoll_create(1); // Parameter is ignored
	// Init listen socket
	ListenSocket	lsocket(8080); // For now we only have one listening socket. Later we will have one per interface:port pair.
	if (epoll_fd < 0) // SHOULD HAVE AN EPOLL CLASS
		return 1;
	// Main loop
	while (1) // TODO: Handle SIGKILL with global to stop loop and exit cleanly
	{
		// 1. Epoll (listen sockets, readable clients always, sendable clients if needed)
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
	return 0;
}

// epoll thoughts: (THESE THOUGHTS DO NOT TAKE MULTI-THREADING INTO ACCOUNT)
//
//   epoll_create(1)
// This returns epoll_fd and we need that for all epoll actions: epoll_ctl and epoll_wait.
//
// We need to use epoll_ctl every time we want to register or unregister a new fd and its flags:
//  a. On listen socket creation (EPOLLIN)
//  b. Whenever we have reached client connection cap (unregister all listen socket fds)
//      (if we don't do that, epoll_wait will insta return and we will use 100% CPU)
//  c. Whenever we have free client connection slots again (register all listen socket fds)
//  d. On client socket creation (EPOLLIN)
//  e. On client reaching responding state (EPOLLOUT and unregister EPOLLIN)
//  f. On client reaching reading state (EPOLLIN and unregister EPOLLOUT)
// No need to unregister an fd when we close a socket - this is done automatically (ignoring multi-threading).
//
// We also need epoll_fd for epoll_wait (but this is done in main loop and is fine)
//
// The main problem is that we need to send epoll_fd to socket classes.
// We will handle multi-threading when we get to it.


// Next step:
//
// 1. Create loop to receive & send several times
// 2. Use epoll to check for new client connections
// 3. Make a container for client connections
// 4. Make a socket class
// 5. For now, ignore the request receive, just read into a buffer until 0 or \r\n\r\n like I said, but ignore the buffer
// 6. After the request is complete (we're not building it yet), reply with generic response, nothing is happening yet.

// Thoughts:
//
// Make sure to use
// signal(SIGPIPE, SIG_IGN)
// At startup eventually, since sending to a closed connection will raise SIGPIPE which by default kills the program
//
// Register EPOLLOUT only if there is a response to send (this will depend on connection status)
// Register EPOLLIN when there is data to receive
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
// Q: I made an accept method for my ListenSocket but. What if my epoll says my ListenSocket has a pending connection, but then
//  by the time that I accept, the pending connection's gone?
// A: Then the accept fails and we continue execution
//
// Q: What if my epoll finds that a listen socket has several pending connections - do I accept in a loop or do I accept ONCE
//  and then run another epoll_wait?
//
// Q: Do we read additional requests in advance while responding? Do we build a queue of requests for one connection?
// A: No. Gains too little, unnecessary. Build a state machine: READING_REQUEST -> (complete request parsed) -> WRITING_RESPONSE
//     Only start reading the next request after having sent all of the response. Note that the next request might (fully) live
//     inside of the buffer, so check the buffer for a full request first. Also unregister the read event from epoll until we
//     actually want to read again. This makes for a clean design.


// Ideas on what a full prog might look like
//
// 1. Attempt to open configuration file and load configuration
//    In case of invalid config file, load default config
// 2. Setup prog: signal handlers, sockets bound to correct ports, listen
//    Any failure prevents the server from working properly so should print an error and abort
// 3. Main loop:
//    a. Check global "stop" set by Ctrl+C - exit if stop is set
//    b. Poll ALL the sockets using epoll_wait (with a timeout for idle connection cleaning)
//    c. If any listen sockets are active, accept() to open new communication channels
//    d. If any active sockets can be read from, read them (see below)
//    e. If any active sockets can be written to, write to them (see below)
//    f. Clean any idle connections
// 4. Cleanup: close sockets, free heap memory
//
// Note that all of the "setup" (e.g config file, signal handler, socket) should be automated by constructors.
// And all of the "cleanup" should be automated by destructors.
//
// Details on epoll results:
//
// We poll listen sockets for new connections, but we also, in the same epoll (as per subject guidelines), poll all the active
//  connection sockets to know whether we can read from or write to them.
// We might not always want to write, but we always want to read.
// Here is the order in which we should handle a new connection (which decides what to do in points d. and e.):
// 1. New connection just happened, create the object with accept() and set the O_NONBLOCK flag. Add the socket to epoll's list.
// 2. Whenever epoll returns that the socket's readable, read in a loop. Stop on 0 read, or reaching \r\n\r\n.
//    a. If we reach \r\n\r\n, fill the request from the buffer. The request might be POST and contain content-length.
//       This would prompt further reading until we have read content-length bytes.
//    b. If we reach 0, stop there and continue with the main loop - it just means we received an incomplete request and will
//        finish reading it later (or send an incomplete request error).
// 3. When a request is complete, process it into a response. We are now polling whether we can write to the socket.
// 4. Whenever epoll returns that the socket's writeable, write in a loop.
//    Only stop on a 0 write, or the message being completely sent. In the case of a 0 write, stop there and continue with main loop,
//     exactly like read: we will finish writing later.
//
// Note that we can read a new request while sending the response from the previous one. We just won't respond until we are done
//  sending the current response.
// Browsers should normally open several channels for several GET requests.
//
// Also note that a connection might close at any moment. Not sure how to handle that yet, but we should just clean the connection.
