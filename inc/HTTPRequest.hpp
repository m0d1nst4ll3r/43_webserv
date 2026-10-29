#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

// Incomplete class - WIP
//
// This class represents an HTTP Request
// It holds all relevant info about the request
// It holds its method, path, protocol, and all header fields
// It is constructed from an input string - construction will parse the string and throw on error
// Errors detected at construction only include parsing errors (e.g a malformed line)
// (e.g a request that starts with "GET / HTTP/1.1 extragarbage")
// These errors will cause a 400 response
// Other types of errors can be checked with methods
//
// Errors will cause a throw for a custom exception
// This custom exception will include extra data, like the error code, and any relevant info
// (to be decided later)
class HTTPRequest
{
	public:

	// Construct from input - will throw for any bad input (custom exception)
	HTTPRequest(const std::string& input);
	~HTTPRequest();

	private:

	std::string							_method;
	std::string							_path;
	std::string							_protocol;
	std::map<std::string, std::string>	_headers;
};

#endif /* HTTPREQUEST_HPP */