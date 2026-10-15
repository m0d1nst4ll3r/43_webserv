#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <arpa/inet.h>
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
			throw std::runtime_error("Read fail: " + std::string(strerror(errno)));
		msg.append(buf, ret);
	} while (ret && msg.find("\r\n\r\n") == std::string::npos);
	return msg;
}

// Open fresh inactive socket, return its fd
// Also set SO_REUSEADDR to avoid "Address already in use" errors
int	openSocket()
{
	int	sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		throw std::runtime_error("socket failed: " + std::string(strerror(errno)));
	const int	enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		close(sockfd);
		throw std::runtime_error("setsockopt failed: " + std::string(strerror(errno)));
	}
	return sockfd;
}

// Bind socket to port
void	bindSocket(int sockfd, unsigned short port)
{
	struct sockaddr_in	sa;

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0)
		throw std::runtime_error("bind failed: " + std::string(strerror(errno)));
}

// Listen to socket
void	listenSocket(int sockfd)
{
	if (listen(sockfd, SOMAXCONN) < 0)
		throw std::runtime_error("listen failed: " + std::string(strerror(errno)));
}

// Listen & wait for connection, ignore connection origin
int	waitForMsg(int sockfd)
{
	int	acceptfd;
	acceptfd = accept(sockfd, 0, 0);
	if (acceptfd < 0)
		throw std::runtime_error("accept failed: " + std::string(strerror(errno)));
	return acceptfd;
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
	int					sockfd = -1;
	int					acceptfd = -1;

	try
	{
		// Open
		std::cout << "\033[33m1:\033[0m Opening new socket... ";
		sockfd = openSocket();
		std::cout << "\033[33mDone\033[0m\n";

		// Bind
		std::cout << "\033[33m2:\033[0m Binding new socket to port 8080... ";
		bindSocket(sockfd, 8080);
		std::cout << "\033[33mDone\033[0m\n";

		// Listen
		std::cout << "\033[33m3:\033[0m Changing new socket state to listening... ";
		listenSocket(sockfd);
		std::cout << "\033[33mDone\033[0m\n";

		// Wait & Print
		std::cout << "\033[33m4:\033[0m Waiting... " << std::flush;
		acceptfd = waitForMsg(sockfd);
		std::string	msg = receiveMsg(acceptfd);
		if (!msg.empty())
		{
			std::cout << "\033[31mMessage received:\033[0m\n";
			std::cout << "\033[36m" << msg << "\033[0m";
		}
		else
			std::cout << "\033[31mNo message\033[0m\n";

		// Reply "Hello world!"
		std::cout << "\033[33m5:\033[0m Replying... ";
		reply(acceptfd, "Hello, world!\n");
		std::cout << "\033[33mDone\033[0m\n";

		// Close
		close(acceptfd);
		close(sockfd);
	}
	catch (const std::exception& e)
	{
		if (acceptfd >= 0)
			close(acceptfd);
		if (sockfd >= 0)
			close(sockfd);
		std::cerr << "\n\033[31mError:\033[0m " << e.what() << "\n";
	}
}

int	main()
{
	socket_test();
}
