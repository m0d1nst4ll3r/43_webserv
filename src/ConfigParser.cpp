/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:25 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/26 18:31:55 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>
#include <limits>

#include <ConfigParser.hpp>

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

    if (path.empty() || path[path.size() - 1] != ';')
        throw std::runtime_error("Error: missing final semicolon.");

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
	std::size_t first = line.find_first_of("0123456789");

    if (first == std::string::npos)
		throw std::runtime_error("Error: invalid body size.");
	
	std::size_t end = line.find_last_not_of(" \t\r\n");
    
	if (end == std::string::npos || line[end] != ';')
        throw std::runtime_error("Error: missing final semicolon.");

    if (end > 0 && line[end - 1] == ';')
		throw std::runtime_error("Error: multiple final semicolons.");

	std::string value = line.substr(first, end - first);

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

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            throw std::runtime_error("Error: invalid body size.");
    }
	
	std::stringstream bodyStream(value);
	unsigned long bodyValue;
	
	if (!(bodyStream >> bodyValue))
        throw std::runtime_error("Error: invalid body size.");
	if (bodyValue > std::numeric_limits<unsigned long>::max() / multiplier)
        throw std::runtime_error("Error: body size too large.");

	config.maxBodySize = bodyValue * multiplier;
}

void push_string(std::string& config, const std::string& line, const std::string& name)
{
	std::size_t first = line.find(name) + name.size();
	
	std::string directive_line = line.substr(first);

	std::size_t end = directive_line.find_last_not_of(" \t\r\n");
    if (end == std::string::npos || directive_line[end] != ';')
        throw std::runtime_error("Error: missing final semicolon.");

    if (end > 0 && directive_line[end - 1] == ';')
		throw std::runtime_error("Error: multiple final semicolons.");

	std::size_t debut = directive_line.find_first_not_of(" \t");
    
	if (debut == std::string::npos || debut >= end)
		throw std::runtime_error("Error: no " + name + ".");
	
	std::size_t last = directive_line.find_last_not_of(" \t;\r\n");
	
	std::string	directive = directive_line.substr(debut, last - debut + 1);
	
	if (directive.find(" ") != std::string::npos || directive.find("\t") != std::string::npos || directive.find(";") != std::string::npos)
		throw std::runtime_error("Error: invalid " + name + ".");
    
	config = directive;
}

void push_listen(Config& config, const std::string& line)
{
    ListenAddress listen;

    std::size_t first;
    std::size_t i = 0;

    for (; i < line.size(); i++)
    {
        if (std::isdigit(static_cast<unsigned char>(line[i])))
        {
            first = i;
            break;
        }
    }

    if (i == line.size())
        throw std::runtime_error("Error: wrong listen format.");

    std::size_t last = line.find(':', first);
    if (last == std::string::npos || last == first)
        throw std::runtime_error("Error: wrong listen format.");

    std::string interface = line.substr(first, last - first);

    int points = 0;
    for (std::string::size_type n = 0; n < interface.size(); n++)
    {
        if (!std::isdigit(static_cast<unsigned char>(interface[n]))
            && interface[n] != '.')
            throw std::runtime_error("Error: wrong character in the interface.");

        if (interface[n] == '.')
        {
            if (n == 0 || n == interface.size() - 1)
                throw std::runtime_error("Error: wrong interface format.");

            if (!std::isdigit(static_cast<unsigned char>(interface[n - 1]))
                || !std::isdigit(static_cast<unsigned char>(interface[n + 1])))
                throw std::runtime_error("Error: wrong interface format.");

            points++;
        }
    }

    if (points != 3)
        throw std::runtime_error("Error: wrong interface format.");

    std::stringstream ipStream(interface);
    std::string octet;
    int octetValue;
    int count = 0;

    while (std::getline(ipStream, octet, '.'))
    {
        std::stringstream octetStream(octet);
        char extra;

        if (octet.empty() || !(octetStream >> octetValue) || (octetStream >> extra))
            throw std::runtime_error("Error: invalid IP octet.");

        if (octetValue < 0 || octetValue > 255)
            throw std::runtime_error("Error: IP octet out of range.");
        count++;
    }

    if (count != 4)
        throw std::runtime_error("Error: wrong interface format.");

    listen.interface = interface;

    std::size_t portStart = last + 1;
    std::size_t semicolon = line.find(';', portStart);

    if (semicolon == std::string::npos)
        throw std::runtime_error("Error: missing semicolon.");

    std::string port = line.substr(portStart, semicolon - portStart);

    if (port.empty())
        throw std::runtime_error("Error: missing port.");

    std::stringstream portStream(port);
    int truePort;
    char extra;

    if (!(portStream >> truePort) || (portStream >> extra))
        throw std::runtime_error("Error: wrong port format.");

    if (truePort < 1 || truePort > 65535)
        throw std::runtime_error("Error: wrong port value.");

    listen.port = truePort;
    config.listens.push_back(listen);
}

std::vector<Config> ConfigParser::parse(const std::string& filename)
{
	std::ifstream file(filename.c_str());
    
	if (!file.is_open())
		throw std::runtime_error("Error: could not open database.");

	std::vector<Config> configs;

	bool server = false;
	bool location = false;
	std::string	line;
	
	Config config;
	LocationConfig location_config;
	
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
            location = true;
            continue;
        }

		 if (clean_line == "}")
        {
            if (location)
            {
                config.locations.push_back(location_config);
                location = false;
                continue;
            }

            configs.push_back(config);
            server = false;
            continue;
        }
		
		if (location)
		{
			// push_location_path(location_config, clean_line);
			// push_allowed_methods(location_config, clean_line);
			// push_autoindex(location_config, clean_line);
			// push_index(location_config, clean_line);
			// push_root(location_config, clean_line);
			// push_upload_enabled(location_config, clean_line);
			// push_upload_store(location_config, clean_line);
			// push_redirect(location_config, clean_line);
			// push_cgi_extension(location_config, clean_line);
			// push_cgi_path(location_config, clean_line);
		}
		else
		{
			if (isDirective(clean_line, "listen"))
				push_listen(config, clean_line);
			else if (isDirective(clean_line, "server_name"))
				push_string(config.serverName, clean_line, "server_name");
			else if (isDirective(clean_line, "root"))
				push_string(config.root, clean_line, "root");
			else if (isDirective(clean_line, "index"))
				push_string(config.index, clean_line, "index");
			else if (isDirective(clean_line, "client_max_body_size"))
				push_body_size(config, clean_line);
			else if (isDirective(clean_line, "error_page"))
				push_error_page(config, clean_line);
		}
	}

	if (server || location)
		throw std::runtime_error("Error: unclosed block.");

    if (configs.empty())
		throw std::runtime_error("Error: no server config.");
		
	return (configs);
}