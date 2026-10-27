#include "ServerConfig.hpp"
#include <string>
#include <map>
#include <stdexcept>

ServerConfig::ServerConfig()
{
	_config["myConfigKey"] = "configValue"; // Example config field
}

ServerConfig::~ServerConfig() {}

const std::string&	ServerConfig::operator[](const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _config.find(key);
	if (it == _config.end())
		throw std::out_of_range("ServerConfig: unknown key '" + key + "'");
	return it->second;
}