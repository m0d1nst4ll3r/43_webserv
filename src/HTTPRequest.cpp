#include "HTTPRequest.hpp"
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

// WIP
HTTPRequest::HTTPRequest(const std::string& input, const std::vector<std::string>& legalMethods, const std::vector<std::string>& legalProtocols)
: _method(), _path(), _protocol(), _headers()
{
	std::size_t first = input.find_first_of("\r\n");

	std::string first_line = input.substr(0, first);

	requestline_parser(first_line, legalMethods, legalProtocols);
	headers_parser();
	
	// Parsing code to fill method (GET), path (/), protocol (HTTP/1.1) and headers
	// Input has to be exactly:
	// "METHOD PATH PROTOCOL"		Only one space allowed (2 total)
	// "KEY1: VALUE1"				Multiple spaces/tabs allowed (before or after VALUE) -> should trim spaces/tabs around VALUE
	// ...							No spaces in KEY
	// "KEYn: VALUEn"				Any character is possible inside VALUE
	// ""							(empty line)
	// Any exception to these rules should throw an..... exception
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::requestline_parser(const std::string& line, const std::vector<std::string>& legalMethods, const std::vector<std::string>& legalProtocols)
{
	std::istringstream stream(line);

	std::string	method;
	std::string	path;
	std::string	protocol;
	std::string	extra;

	if (!(stream >> method >> path >> protocol))
		throw std::runtime_error("Error: invalid request-line.");

	if (stream >> extra)
		throw std::runtime_error("Error: too many arguments in request-line.");

	std::vector<std::string>::const_iterator it;
	for (it = legalMethods.begin(); it != legalMethods.end(); ++it)
	{
		if (*it == method)
			break;
	}
	if (it == legalMethods.end())
		throw std::runtime_error("Error: wrong method in request-line.");

	if (path.empty() || path[0] != '/')
    	throw std::runtime_error("Error: wrong path in request-line.");
	
	for (std::size_t i = 0; i < path.size(); ++i)
	{
		unsigned char character = static_cast<unsigned char>(path[i]);

		if (character < 0x20 || character == 0x7f)
			throw std::runtime_error("Error: invalid character in path.");

		if (path[i] == '%')
		{
			if (i + 2 >= path.size()
				|| !isHexDigit(path[i + 1])
				|| !isHexDigit(path[i + 2]))
			{
				throw std::runtime_error("Error: invalid percent encoding.");
			}

			i += 2;
		}
	}

	std::vector<std::string>::const_iterator ite;
	for (ite = legalProtocols.begin(); ite != legalProtocols.end(); ++ite)
	{
		if (*ite == protocol)
			break;
	}
	if (ite == legalProtocols.end())
		throw std::runtime_error("Error: wrong protocol in request-line.");

	_method = method;
	_path = path;
	_protocol = protocol;
}

bool	HTTPRequest::isHexDigit(char character)
{
    return (character >= '0' && character <= '9')
        || (character >= 'a' && character <= 'f')
        || (character >= 'A' && character <= 'F');
}