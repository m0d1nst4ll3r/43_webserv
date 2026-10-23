/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:25 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/23 17:38:44 by ltourbe          ###   ########.fr       */
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

std::vector<Config> ConfigParser::parse(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	bool server = false;
	bool location = false;

    if (!file.is_open())
        throw std::runtime_error("Error: could not open database.");
	
	std::string	line;
	std::getline(file, line);
	
	while (line.find("server {") == std::string::npos)
		std::getline(file, line);

	if (server == true)
		Config config;
	else
		throw std::runtime_error("Error: no server config.");
	
	while (std::getline(file, line))
	{
		if (line.find("location /"))
		{
			server = false;
			location = true;
			LocationConfig location_config;
		}
		if (line.find("listen") != std::string::npos && server)
			push_listen(config, line);
		else if (line.find("server_name") != std::string::npos && server)
			push_server_name(config, line);
		else if (line.find("root") != std::string::npos && server)
			push_root(config, line);
		else if (line.find("index") != std::string::npos && server)
			push_index(config, line);
		else if (line.find("client_max_body_size") != std::string::npos && server)
			push_body_size(config, line);
		else if (line.find("error") != std::string::npos && server)
			push_error(config, line);
		else if (line.find("/") != std::string::npos && location)
			push_location_path(location_config, line);
		else if (line.find("/") != std::string::npos && location)
	}
	
	}



		
	std::getline(file, line);
	
	while (std::getline(file, line) || !line.find("server {"))
	{		
		if (line.find("listen"))
			push_listen();
		else if (line.find("server_name)")
			push_server_name();
		else if (line.find("server_name)"))
	}
}