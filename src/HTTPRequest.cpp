#include "HTTPRequest.hpp"
#include <string>
#include <map>

// WIP
HTTPRequest::HTTPRequest(const std::string& input)
: _method(), _path(), _protocol(), _headers()
{
	(void)input;
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