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

// Read from socket, only stop when finding \r\n\r\n or running out of bytes to read
std::string	receiveMsg(int sockfd)
{
	char		buf[64];
	int			ret;
	std::string	msg;

	do
	{
		ret = recv(sockfd, buf, 64, 0);
		if (ret < 0)
			throw std::runtime_error("Read fail: " + std::string(std::strerror(errno)));
		msg.append(buf, ret);
	} while (ret && msg.find("\r\n\r\n") == std::string::npos);
	return msg;
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

// Open a socket, bind it to 8080, listen for a msg and print it
void	socket_test()
{
	int					acceptfd = -1;

	try
	{
		int port = 8080;
		// Open
		std::cout << "\033[33m1:\033[0m Creating new listenSocket on port " << port << "... ";
		ListenSocket		listenSocket(port);
		std::cout << "\033[33mDone\033[0m\n";

		// Wait & Print
		std::cout << "\033[33m2:\033[0m Waiting... " << std::flush;
		acceptfd = listenSocket.accept();
		std::string	msg = receiveMsg(acceptfd);
		if (!msg.empty())
		{
			std::cout << "\033[31mMessage received:\033[0m\n";
			std::cout << "\033[36m" << msg << "\033[0m";
		}
		else
			std::cout << "\033[31mNo message\033[0m\n";

		// Reply "Hello world!"
		std::cout << "\033[33m3:\033[0m Replying... ";
		reply(acceptfd, "Hello, world!\n");
		std::cout << "\033[33mDone\033[0m\n";

		// Close
		close(acceptfd);
	}
	catch (const std::exception& e)
	{
		if (acceptfd >= 0)
			close(acceptfd);
		std::cerr << "\n\033[31mError:\033[0m " << e.what() << "\n";
	}
}

int	main()
{
	socket_test();
}

// Next step:
//
// 1. Create loop to receive & send several times
// 2. Use epoll to check for new client connections
// 3. Make a container for client connections
// 4. Make a socket class
// 5. For now, ignore the request receive, just read into a buffer until 0 or \r\n\r\n like I said, but ignore the buffer
// 6. After the request is complete (we're not building it yet), reply with generic response, nothing is happening yet.


// Questions:
//
// I made an accept method for my ListenSocket but. What if my epoll says my ListenSocket has a pending connection, but then
//  by the time that I accept, the pending connection's gone?


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
