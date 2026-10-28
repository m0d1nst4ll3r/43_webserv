/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:25 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/28 19:04:49 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Utils.hpp"

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}
std::string trim(const std::string& str)
{
	std::size_t first = str.find_first_not_of(" \t\r\n");

	if (first == std::string::npos)
		return "";

	std::size_t last = str.find_last_not_of(" \t\r\n");

	return str.substr(first, last - first + 1);
}

bool isDirective(const std::string& line, const std::string& name)
{
	std::istringstream stream(line);
	std::string directive;

	stream >> directive;

	return directive == name;
}

void push_error_page(Config& config, const std::string& line)
{	
	validate_semicolon(line, "error_page");
	
	std::istringstream stream(line);

	std::string directive;
	std::string codeStr;
	std::string path;
	std::string extra;

	if (!(stream >> directive >> codeStr >> path))
		throw std::runtime_error("Error: invalid error_page directive.");

	if (directive != "error_page")
		throw std::runtime_error("Error: invalid error_page directive.");

	if (stream >> extra)
		throw std::runtime_error("Error: too many arguments in error_page.");

	path.erase(path.size() - 1);

	if (path.empty())
		throw std::runtime_error("Error: missing error page path.");

	if (codeStr.empty())
		throw std::runtime_error("Error: missing HTTP status code.");

	for (std::size_t i = 0; i < codeStr.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(codeStr[i])))
			throw std::runtime_error("Error: invalid HTTP status code.");
	}

	std::istringstream codeStream(codeStr);
	int code;

	if (!(codeStream >> code) || code < 100 || code > 599)
		throw std::runtime_error("Error: invalid HTTP status code.");

	if (config.errorPages.find(code) != config.errorPages.end())
		throw std::runtime_error("Error: duplicate error_page code.");

	config.errorPages[code] = path;
}

void push_body_size(Config& config, const std::string& line)
{
	validate_semicolon(line, "client_max_body_size");

	std::istringstream stream(line);
	std::string directive;
	std::string value;
	std::string extra;

	if (!(stream >> directive >> value))
		throw std::runtime_error("Error: invalid client_max_body_size directive.");

	if (directive != "client_max_body_size")
		throw std::runtime_error("Error: invalid client_max_body_size directive.");

	if (stream >> extra)
		throw std::runtime_error("Error: too many arguments in client_max_body_size.");

	value.erase(value.size() - 1);

	if (value.empty())
		throw std::runtime_error("Error: missing body size.");

	unsigned long multiplier = 1;
	char suffix = value[value.size() - 1];

	if (suffix == 'K' || suffix == 'k')
	{
		multiplier = 1024UL;
		value.erase(value.size() - 1);
	}
	else if (suffix == 'M' || suffix == 'm')
	{
		multiplier = 1024UL * 1024UL;
		value.erase(value.size() - 1);
	}
	else if (suffix == 'G' || suffix == 'g')
	{
		multiplier = 1024UL * 1024UL * 1024UL;
		value.erase(value.size() - 1);
	}

	if (value.empty())
		throw std::runtime_error("Error: invalid body size.");

	unsigned long bodyValue = 0;

	for (std::size_t i = 0; i < value.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(value[i])))
			throw std::runtime_error("Error: invalid body size.");

		unsigned int digit = value[i] - '0';

		if (bodyValue > (std::numeric_limits<unsigned long>::max() - digit) / 10)
			throw std::runtime_error("Error: body size too large.");

		bodyValue = bodyValue * 10 + digit;
	}

	if (bodyValue > std::numeric_limits<unsigned long>::max() / multiplier)
		throw std::runtime_error("Error: body size too large.");

	config.maxBodySize = bodyValue * multiplier;
}

void push_string(std::string& config, const std::string& line, const std::string& name, const std::string& where)
{
	std::size_t first = line.find(name);
	
	if (first == std::string::npos)
		throw std::runtime_error("Error: missing " + where + " " + name + ".");

	first += name.size();
	std::string directive_line = line.substr(first);

	validate_semicolon(directive_line, name);

	std::size_t end = directive_line.find_last_not_of(" \t\r\n");
	
	if (end == std::string::npos || directive_line[end] != ';')
		throw std::runtime_error("Error: " + where + " " + name + ": missing final semicolon.");

	if (end != directive_line.size() - 1)
		throw std::runtime_error("Error: " + where + " " + name + ": whitespace after semicolon.");

	if (directive_line.find(';') != end)
		throw std::runtime_error("Error: " + where + " " + name + ": multiple or misplaced semicolons.");

	std::size_t debut = directive_line.find_first_not_of(" \t");
	
	if (debut == std::string::npos || debut >= end)
		throw std::runtime_error("Error: no " + where + " " + name + ".");
	
	if (end > 0 && (directive_line[end - 1] == ' ' || directive_line[end - 1] == '\t'))
		throw std::runtime_error("Error: " + where + " " + name + ": whitespace before semicolon.");

	std::string	directive = directive_line.substr(debut, end - debut);
	
	if (directive.find(" ") != std::string::npos || directive.find("\t") != std::string::npos || directive.find(";") != std::string::npos)
		throw std::runtime_error("Error: invalid " + where + " " + name + ".");
	
	config = directive;
}

void push_listen(Config& config, const std::string& line)
{
	validate_semicolon(line, "listen");

	ListenAddress listen;

	std::string value = line.substr(0, line.size() - 1);

	std::size_t start = value.find_first_not_of(" \t");
	std::size_t addressStart = value.find_first_of(" \t", start);

	if (addressStart == std::string::npos)
		throw std::runtime_error("Error: missing interface and port.");

	addressStart = value.find_first_not_of(" \t", addressStart);

	if (addressStart == std::string::npos)
		throw std::runtime_error("Error: missing interface and port.");

	std::size_t colon = value.find(':', addressStart);

	if (colon == std::string::npos || colon == addressStart)
		throw std::runtime_error("Error: wrong listen format.");

	std::string interface = value.substr(addressStart, colon - addressStart);
	std::string port = value.substr(colon + 1);

	if (port.empty())
		throw std::runtime_error("Error: missing port.");

	int count = 0;
	std::size_t octetStart = 0;

	while (octetStart <= interface.size())
	{
		std::size_t dot = interface.find('.', octetStart);
		std::string octet = interface.substr(octetStart,
			dot == std::string::npos ? std::string::npos : dot - octetStart);

		if (octet.empty() || octet.size() > 3)
			throw std::runtime_error("Error: invalid IP octet.");

		if (octet.size() > 1 && octet[0] == '0')
			throw std::runtime_error("Error: leading zeros in IP octet.");
		
		int octetValue = 0;

		for (std::size_t i = 0; i < octet.size(); ++i)
		{
			if (!std::isdigit(static_cast<unsigned char>(octet[i])))
				throw std::runtime_error("Error: invalid IP octet.");

			octetValue = octetValue * 10 + (octet[i] - '0');
		}

		if (octetValue > 255)
			throw std::runtime_error("Error: IP octet out of range.");

		++count;

		if (dot == std::string::npos)
			break;

		octetStart = dot + 1;
	}

	if (count != 4)
		throw std::runtime_error("Error: wrong interface format.");

	int truePort = 0;

	for (std::size_t i = 0; i < port.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(port[i])))
			throw std::runtime_error("Error: wrong port format.");

		truePort = truePort * 10 + (port[i] - '0');

		if (truePort > 65535)
			throw std::runtime_error("Error: wrong port value.");
	}

	if (truePort < 1)
		throw std::runtime_error("Error: wrong port value.");

	listen.interface = interface;
	listen.port = truePort;

	config.listens.push_back(listen);
}

std::vector<Config> ConfigParser::parse(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	
	if (!file.is_open())
		throw std::runtime_error("Error: could not open configuration file.");

	std::vector<Config> configs;

	bool server = false;
	bool location = false;
	std::string	line;
	
	Config config;
	LocationConfig location_config;
	
	std::set<std::string> serverDirectives;
	std::set<std::string> locationDirectives;

	while (std::getline(file, line))
	{
		std::size_t comment = line.find('#');
		if (comment != std::string::npos)
			line = line.substr(0, comment);

		std::string clean_line = trim(line);
		
		if (clean_line.empty())
			continue;
		
		if (clean_line == "server {")
		{
			if (server || location)
				throw std::runtime_error("Error: unexpected server block.");

			config = Config();
			server = true;
			serverDirectives.clear();
			continue;
		}
		
		if (!server)
			throw std::runtime_error("Error: directive outside server block.");

		if (clean_line.compare(0, 9, "location ") == 0)
		{
			if (location)
				throw std::runtime_error("Error: nested location block.");

			if (clean_line[clean_line.size() - 1] != '{')
				throw std::runtime_error("Error: invalid location block.");

			location_config = LocationConfig();
			push_location_path(location_config, clean_line);

			for (std::size_t i = 0; i < config.locations.size(); ++i)
			{
				if (config.locations[i].path == location_config.path)
					throw std::runtime_error("Error: duplicate location path.");
			}
			
			location = true;
			locationDirectives.clear();
			continue;
		}

        if (clean_line == "}")
		{
			if (location)
			{
				validate_location_config(location_config);
				config.locations.push_back(location_config);
				location = false;
				continue;
			}

			validate_server_config(config);
			configs.push_back(config);
			server = false;
			continue;
		}
		
		if (location)
		{
			if (isDirective(clean_line, "allowed_methods"))
			{
				checkDuplicate(locationDirectives, "allowed_methods");
				push_allowed_methods(location_config, clean_line);
			}
			else if (isDirective(clean_line, "autoindex"))
			{
				checkDuplicate(locationDirectives, "autoindex");
				push_bool(location_config.autoindex, clean_line, "autoindex");
			}
			else if (isDirective(clean_line, "index"))
			{
				checkDuplicate(locationDirectives, "index");
				push_string(location_config.index, clean_line, "index", "location");
			}
			else if (isDirective(clean_line, "root"))
			{
				checkDuplicate(locationDirectives, "root");
				push_string(location_config.root, clean_line, "root", "location");
			}
			else if (isDirective(clean_line, "upload_enabled"))
			{
				checkDuplicate(locationDirectives, "upload_enabled");
				push_bool(location_config.uploadEnabled, clean_line, "upload_enabled");
			}
			else if (isDirective(clean_line, "upload_store"))
			{
				checkDuplicate(locationDirectives, "upload_store");
				push_string(location_config.uploadStore, clean_line, "upload_store", "location");
			}
			else if (isDirective(clean_line, "return"))
			{
				checkDuplicate(locationDirectives, "return");
				push_redirect(location_config, clean_line);
			}
			else if (isDirective(clean_line, "cgi_extension"))
			{
				checkDuplicate(locationDirectives, "cgi_extension");
				push_string(location_config.cgiExtension, clean_line, "cgi_extension", "location");
			}
			else if (isDirective(clean_line, "cgi_path"))
			{
				checkDuplicate(locationDirectives, "cgi_path");
				push_string(location_config.cgiExecutable, clean_line, "cgi_path", "location");
			}
			else
				throw std::runtime_error("Wrong location directive");
		}
		else
		{
			if (isDirective(clean_line, "listen"))
				push_listen(config, clean_line);
			else if (isDirective(clean_line, "server_name"))
			{
				checkDuplicate(serverDirectives, "server_name");
				push_string(config.serverName, clean_line, "server_name", "server");
			}
			else if (isDirective(clean_line, "root"))
			{
				checkDuplicate(serverDirectives, "root");
				push_string(config.root, clean_line, "root", "server");
			}
			else if (isDirective(clean_line, "index"))
			{
				checkDuplicate(serverDirectives, "index");
				push_string(config.index, clean_line, "index", "server");
			}
			else if (isDirective(clean_line, "client_max_body_size"))
			{
				checkDuplicate(serverDirectives, "client_max_body_size");
				push_body_size(config, clean_line);
			}
			else if (isDirective(clean_line, "error_page"))
				push_error_page(config, clean_line);
			else
				throw std::runtime_error("Wrong server directive");
		}
	}

	if (server || location)
		throw std::runtime_error("Error: unclosed block.");

	if (configs.empty())
		throw std::runtime_error("Error: no server config.");
		
	return (configs);
}
