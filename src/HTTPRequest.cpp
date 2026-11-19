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
	std::string	headers = input.substr(first + 2);

	requestline_parser(first_line, legalMethods, legalProtocols);
	headers_parser(headers);
	
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

bool	isTokenCharacter(char i)
{
	if ((i >= '0' && i <= '9') || (i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z')
		|| i == '!' || (i >= '#' && i <= 39) || i == '*' || i == '+' || i == '-'
		|| i == '.' || i == '^' || i == '_' || i == '`' || i == '|' || i == '~')
		return true;

	return false;
}

void	HTTPRequest::headers_parser(const std::string& headers)
{	
	std::istringstream stream(headers);
	std::string	line;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty())
        	break;

		std::size_t colon = line.find(':');

		if (colon == std::string::npos || colon == 0)
			throw std::runtime_error("Error: invalid header.");

		std::string name = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		if (name.empty())
			throw std::runtime_error("Error: empty header name.");

		for (std::size_t i = 0; i < name.size(); ++i)
		{
			if (!isTokenCharacter(name[i]))
				throw std::runtime_error("Error: invalid character in header name.");
			name[i] = std::tolower(static_cast<unsigned char>(name[i]));
		}
		
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);

		while (!value.empty() && (value[value.size() - 1] == ' ' || value[value.size() - 1] == '\t'))
			value.erase(value.size() - 1, 1);

		for (std::size_t i = 0; i < value.size(); ++i)
		{
			unsigned char character = static_cast<unsigned char>(value[i]);

			if ((character < 0x20 && character != '\t') || character == 0x7f)
				throw std::runtime_error("Error: invalid header value.");
		}

		std::map<std::string, std::string>::iterator existing;
		existing = _headers.find(name);

		if (existing != _headers.end())
		{
			if (name == "content-length" && existing->second == value)
			{
				continue;
			}
			else
			{
				throw std::runtime_error("Error: duplicate header.");
			}
		}
		else
		{
			_headers[name] = value;
		}
	}
	
	if (_protocol == "HTTP/1.1")
	{
		std::map<std::string, std::string>::const_iterator host;
		host = _headers.find("host");

		if (host == _headers.end() || host->second.empty())
			throw std::runtime_error("Error: missing Host header.");
	}
}

void	HTTPRequest::requestline_parser(const std::string& line, const std::vector<std::string>& legalMethods, const std::vector<std::string>& legalProtocols)
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

const std::string& HTTPRequest::getMethod() const
{
	return _method;
}

const std::string& HTTPRequest::getPath() const
{
	return _path;
}

const std::string& HTTPRequest::getProtocol() const
{
	return _protocol;
}

bool	HTTPRequest::hasHeader(const std::string& name) const
{
	return _headers.find(name) != _headers.end();
}

const std::string& HTTPRequest::getHeader(const std::string& name) const
{
	static const std::string empty;

	std::map<std::string, std::string>::const_iterator it =
		_headers.find(name);

	if (it == _headers.end())
		return empty;

	return it->second;
}