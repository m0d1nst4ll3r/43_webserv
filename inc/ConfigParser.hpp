/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:13 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/28 18:54:37 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include "Config.hpp"

class ConfigParser
{
	public:
    
		ConfigParser();
		~ConfigParser();

		std::vector<Config> parse(const std::string& filename);

		void push_location_path(LocationConfig& location, const std::string& line);
		void push_allowed_methods(LocationConfig& location, const std::string& line);
		void push_bool(bool& location, const std::string& line, const std::string& name);
		void push_redirect(LocationConfig& location, const std::string& line);

};

#endif