/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:39:14 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/27 17:52:46 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include "ConfigParser.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv config.conf" << std::endl;
        return 1;
    }

    try
    {
        ConfigParser parser;
        std::vector<Config> configs = parser.parse(argv[1]);

        std::cout << "Nombre de serveurs : "
                  << configs.size() << std::endl;

        for (std::size_t i = 0; i < configs.size(); ++i)
        {
            const Config &config = configs[i];

            std::cout << "\n========== SERVER "
                      << i + 1 << " ==========" << std::endl;

            for (std::size_t j = 0; j < configs[i].listens.size(); ++j)
            {
                std::cout << "IP : "
                        << configs[i].listens[j].interface
                        << std::endl;

                std::cout << "Port : "
                        << configs[i].listens[j].port
                        << std::endl;
            }
            std::cout << "Server name : "
                      << config.serverName << std::endl;
            std::cout << "Root : "
                      << config.root << std::endl;
            std::cout << "Index : "
                      << config.index << std::endl;
            std::cout << "Max body size : "
                      << config.maxBodySize << std::endl;

            std::cout << "Nombre de locations : "
                      << config.locations.size() << std::endl;

            std::cout << "Error pages :" << std::endl;

            for (std::map<int, std::string>::const_iterator it =
                    config.errorPages.begin();
                it != config.errorPages.end(); ++it)
            {
                std::cout << "  HTTP " << it->first
                        << " -> " << it->second << std::endl;
            }

            for (std::size_t y = 0;
                  y < config.locations.size(); ++y)
            {
                const LocationConfig &loc = config.locations[y];

                std::cout << "\n  ----- LOCATION "
                          << y + 1 << " -----" << std::endl;

                std::cout << "  Path : "
                          << loc.path << std::endl;
                std::cout << "  Root : "
                          << loc.root << std::endl;
                std::cout << "  Index : "
                          << loc.index << std::endl;
                std::cout << "  Autoindex : "
                          << (loc.autoindex ? "on" : "off")
                          << std::endl;
                std::cout << "  Upload enabled : "
                          << (loc.uploadEnabled ? "on" : "off")
                          << std::endl;

                std::cout << "  Allowed methods : ";
                for (std::size_t k = 0;
                     k < loc.allowedMethods.size(); ++k)
                {
                    std::cout << loc.allowedMethods[k] << " ";
                }
                std::cout << std::endl;

                std::cout << "  Upload store : "
                          << loc.uploadStore << std::endl;
                std::cout << "  Redirect code : "
                          << loc.redirectCode << std::endl;
                std::cout << "  Redirect : "
                          << loc.redirect << std::endl;
                std::cout << "  CGI extension : "
                          << loc.cgiExtension << std::endl;
                std::cout << "  CGI path : "
                          << loc.cgiExecutable << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}