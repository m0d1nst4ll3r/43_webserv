/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: leo <leo@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:25 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/24 00:46:11 by leo              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

#include <ConfigParser.hpp>

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}

bool isDirective(const std::string& line, const std::string& name)
{
    std::istringstream stream(line);
    std::string directive;

    stream >> directive;

    return directive == name;
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
		throw std::runtime_error("Error: no " + name + " .");
	
	std::size_t last = directive_line.find_last_not_of(" \t;\r\n");
	
	std::string	directive = directive_line.substr(debut, last - debut + 1);
	
	if (directive.find(" ") != std::string::npos || directive.find("\t") != std::string::npos || directive.find(";") != std::string::npos)
		throw std::runtime_error("Error: invalid " + name + " .");
    
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
	std::getline(file, line);
	
	while (line.find("server {") == std::string::npos)
	{
		if (!std::getline(file, line))
			throw std::runtime_error("Error: no server config.");
	}
			
	server = true;
	Config config;
	LocationConfig location_config;
	
	while (std::getline(file, line))
	{
		if (line.find("server {")!= std::string::npos)
		{
			server = true;
			config = Config();
		}
		if (line.find("location /")!= std::string::npos)
		{
			server = false;
			location = true;
			location_config = LocationConfig();
		}
		if (server)
		{
			if (isDirective(line, "listen"))
				push_listen(config, line);
			else if (isDirective(line, "server_name"))
				push_string(config.server_name, line, "server_name");
			else if (isDirective(line, "root"))
				push_string(config.root, line, "root");
			else if (isDirective(line, "index"))
				push_string(config.index, line, "index");
			else if (isDirective(line, "client_max_body_size"))
				push_body_size(config, line);
			else if (isDirective(line, "error"))
				push_error(config, line);
			else if (line.find("}") != std::string::npos)
			{
				configs.push_back(config);
				server = false;
			}
		}
		else if (location)
		{
			if (line.find("location") != std::string::npos) 
				push_location_path(location_config, line);
			else if (line.find("allowed_methods") != std::string::npos)
				push_allowed_methods(location_config, line);
			else if (line.find("autoindex") != std::string::npos)
				push_autoindex(location_config, line);
			else if (line.find("index") != std::string::npos)
				push_index(location_config, line);
			else if (line.find("root") != std::string::npos)
				push_root(location_config, line);
			else if (line.find("upload_enabled") != std::string::npos)
				push_upload_enabled(location_config, line);
			else if (line.find("upload_store") != std::string::npos)
				push_upload_store(location_config, line);
			else if (line.find("return") != std::string::npos)
				push_redirect(location_config, line);
			else if (line.find("cgi_extension") != std::string::npos)
				push_cgi_extension(location_config, line);
			else if (line.find("cgi_path") != std::string::npos)
				push_cgi_path(location_config, line);
			else if (line.find("}") != std::string::npos)
			{
				location = false;
				server = true;
				config.locations.push_back(location_config);
			}
		}
	}
	return (configs);
}