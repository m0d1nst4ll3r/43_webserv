#include "HTTPResponse.hpp"
#include <sstream>

HTTPResponse::HTTPResponse(int code)
: _protocol("HTTP/1.1"),
  _statusCode(),
  _statusStr(),
  _headers(),
  _body()
{
    setStatusCode(code);
}

HTTPResponse::HTTPResponse(const HTTPRequest& request)
: _protocol("HTTP/1.1"),
  _statusCode(),
  _statusStr(),
  _headers(),
  _body()
{
    (void)request;
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::setStatusCode(int code)
{
    std::ostringstream stream;
    stream << code;
    _statusCode = stream.str();
}

void HTTPResponse::setReasonPhrase(const std::string& reason)
{
    _statusStr = reason;
}

void HTTPResponse::setHeader(
    const std::string& name,
    const std::string& value)
{
    _headers[name] = value;
}

void HTTPResponse::setBody(const std::string& body)
{
    _body = body;
}

std::string HTTPResponse::toString() const
{
    std::ostringstream response;

    response << _protocol << " "
            << _statusCode << " "
            << _statusStr << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
        it != _headers.end();
        ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }

    response << "Content-Length: " << _body.size() << "\r\n";
    response << "\r\n";
    response << _body;

    return response.str();
}