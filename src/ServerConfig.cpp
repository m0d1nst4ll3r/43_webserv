#include "ServerConfig.hpp"
#include <string>
#include <vector>

ServerConfig::ServerConfig()
:	_keepAliveTime(10),
	_requestHeaderTime(10),
	_requestBodyTime(10),
	_responseTime(10),
	_maxConnections(256),
	_maxWebsites(32),
	_requestLineMaxLength(8000),
	_requestHeaderMaxLength(32000),
	_requestBodyDfltLength(10000000),
	_requestBodyMaxLength(50000000),
	_headerLineMaxLength(8000),
	_CGIResponseMaxLength(64000000),
	_serverProtocols(),
	_serverMethods(),
	_legalProtocols(),
	_legalMethods()
{
	_serverProtocols.push_back("HTTP/1.1");
	_serverProtocols.push_back("HTTP/1.0");
	_serverMethods.push_back("GET");
	_serverMethods.push_back("POST");
	_serverMethods.push_back("DELETE");
	_legalProtocols.push_back("HTTP/0.9");
	_legalProtocols.push_back("HTTP/1.0");
	_legalProtocols.push_back("HTTP/1.1");
	_legalProtocols.push_back("HTTP/2");
	_legalProtocols.push_back("HTTP/3");
	_legalMethods.push_back("GET");
	_legalMethods.push_back("POST");
	_legalMethods.push_back("DELETE");
	_legalMethods.push_back("HEAD");
	_legalMethods.push_back("PATCH");
	_legalMethods.push_back("OPTIONS");
	_legalMethods.push_back("CONNECT");
	_legalMethods.push_back("PUT");
	_legalMethods.push_back("QUERY");
	_legalMethods.push_back("TRACE");
}

ServerConfig::~ServerConfig() {}