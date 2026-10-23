/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:39:14 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/23 16:39:15 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "LocationConfig.hpp"
#include <iostream>

int main()
{
    Config config;
    config.root = "./www";
    config.index = "index.html";
    config.maxBodySize = 10000000;

    LocationConfig location;
    location.path = "/contact";
    location.root = "./www";
    location.allowedMethods.push_back("GET");

    config.locations.push_back(location);

    std::cout << config.root << std::endl;
    std::cout << config.locations[0].path << std::endl;
    std::cout << config.locations[0].allowedMethods[0] << std::endl;

    return 0;
}