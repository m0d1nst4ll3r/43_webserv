#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>

// Holds all server config in a string:string map
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

	// Fetch config string
	const std::string&	operator[](const std::string& key) const;

	private:

	std::map<std::string, std::string>	_config;
};

#endif /* SERVERCONFIG_HPP */