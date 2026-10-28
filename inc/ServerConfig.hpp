#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>

struct ServerConfig
{
	// All time values are in seconds
	unsigned long	keepAliveTime;			// How long to keep inactive client session alive
	unsigned long	requestHeaderTime;		// How long to keep reading request headers
	unsigned long	requestBodyTime;		// How long to keep reading request body
	unsigned long	responseTime;			// How long to keep sending a response
	unsigned long	maxConnections;			// How many concurrent connections max
	unsigned long	maxWebsites;			// How many websites max
	unsigned long	requestLineMaxLength;	// Max length for request line (first line)
	unsigned long	requestHeaderMaxLength;	// Max length for request headers section
	unsigned long	requestBodyDfltLength;	// Default request body length
	unsigned long	requestBodyMaxLength;	// Max length for request body
	unsigned long	headerLineMaxLength;	// Max length for a single header line
	unsigned long	CGIResponseMaxLength;	// Max length for CGI response
	std::vector<std::string>	serverProtocols;// What protocols the server accepts
	std::vector<std::string>	serverMethods;	// What methods the server accepts
	std::vector<std::string>	legalProtocols;	// What protocols exist
	std::vector<std::string>	legalMethods;	// What methods exist

	ServerConfig()
	:	keepAliveTime(10),
		requestHeaderTime(10),
		requestBodyTime(10),
		responseTime(10),
		maxConnections(256),
		maxWebsites(32),
		requestLineMaxLength(8000),
		requestHeaderMaxLength(32000),
		requestBodyDfltLength(10000000),
		requestBodyMaxLength(50000000),
		headerLineMaxLength(8000),
		CGIResponseMaxLength(64000000)
	{
		serverProtocols.push_back("HTTP/1.1");
		serverProtocols.push_back("HTTP/1.0");
		serverMethods.push_back("GET");
		serverMethods.push_back("POST");
		serverMethods.push_back("DELETE");
		legalProtocols.push_back("HTTP/0.9");
		legalProtocols.push_back("HTTP/1.0");
		legalProtocols.push_back("HTTP/1.1");
		legalProtocols.push_back("HTTP/2");
		legalProtocols.push_back("HTTP/3");
		legalMethods.push_back("GET");
		legalMethods.push_back("POST");
		legalMethods.push_back("DELETE");
		legalMethods.push_back("HEAD");
		legalMethods.push_back("PATCH");
		legalMethods.push_back("OPTIONS");
		legalMethods.push_back("CONNECT");
		legalMethods.push_back("PUT");
		legalMethods.push_back("QUERY");
		legalMethods.push_back("TRACE");
	}
};

#endif /* SERVERCONFIG_HPP */