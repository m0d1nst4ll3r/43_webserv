/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: leo <leo@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:39:14 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/23 23:46:31 by leo              ###   ########.fr       */
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

            for (std::size_t j = 0;
                 j < config.locations.size(); ++j)
            {
                const LocationConfig &loc = config.locations[j];

                std::cout << "\n  ----- LOCATION "
                          << j + 1 << " -----" << std::endl;

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
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}