#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>

// Holds all server config
//
// Example:
//	- How long to keep a connection alive
//	- How long to wait for sending a response
//	- How many bytes to accept in a request (before body)
//	etc...
class ServerConfig
{
	public:

	ServerConfig();
	~ServerConfig();

	// All time values are in seconds
	unsigned long	_keepAliveTime;			// How long to keep inactive client session alive
	unsigned long	_requestHeaderTime;		// How long to keep reading request headers
	unsigned long	_requestBodyTime;		// How long to keep reading request body
	unsigned long	_responseTime;			// How long to keep sending a response
	unsigned long	_maxConnections;		// How many concurrent connections max
	unsigned long	_maxWebsites;			// How many websites max
	unsigned long	_requestLineMaxLength;	// Max length for request line (first line)
	unsigned long	_requestHeaderMaxLength;// Max length for request headers section
	unsigned long	_requestBodyDfltLength; // Default request body length
	unsigned long	_requestBodyMaxLength;	// Max length for request body
	unsigned long	_headerLineMaxLength;	// Max length for a single header line
	unsigned long	_CGIResponseMaxLength;	// Max length for CGI response
	std::vector<std::string>	_serverProtocols;	// What protocols the server accepts
	std::vector<std::string>	_serverMethods;		// What methods the server accepts
	std::vector<std::string>	_legalProtocols;	// What protocols exist
	std::vector<std::string>	_legalMethods;		// What methods exist

};

#endif /* SERVERCONFIG_HPP */