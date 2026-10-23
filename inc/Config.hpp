/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 18:45:26 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/23 16:35:57 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "LocationConfig.hpp"

struct ListenAddress
{
    std::string interface;

   int port;
};

class Config
{
    public:
        
        Config ();
        ~Config();
        
        std::vector<ListenAddress> listens;
        std::string serverName;
        std::string root;
        std::string index;

        std::map<int, std::string> errorPages;
        unsigned long maxBodySize;

        std::vector<LocationConfig> locations;
};

#endif
