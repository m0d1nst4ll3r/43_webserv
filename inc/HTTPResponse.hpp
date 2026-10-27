#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HTTPRequest.hpp"
#include <string>
#include <map>

// Incomplete class - WIP
//
// This class represents an HTTP Response
// It holds all relevant info about the response
// It holds the protocol, status code, status string, all header fields, and body
//
// Construction takes request and website config to produce a response
// Construction can also take an int code and website config to produce a generic response
//
// Request constructor should be called with a verified valid request
// It will locate the resource, check website config, and build a response (maybe error)
//
// Code constructor should be called when an error happens outside of this scope
// E.g parsing could not even yield a valid Request
//     or, CGI timeout, or request timeout
//     etc...
//
// Note that this suggested plan may break when taking CGI into account... to be refined
class HTTPResponse
{
	public:

	// Construct response from request + website config
	HTTPResponse(int code); // Generic response, WIP - Config class missing
	HTTPResponse(const HTTPRequest& request); // WIP - Config class missing
	~HTTPResponse();

	private:

	std::string							_protocol;
	std::string							_statusCode;
	std::string							_statusStr;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
};

#endif /* HTTPRESPONSE_HPP */