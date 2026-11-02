/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: leo <leo@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:46:13 by ltourbe           #+#    #+#             */
/*   Updated: 2026/11/02 19:47:19 by leo              ###   ########.fr       */
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

		static std::vector<Config> parse(const std::string& filename);

	private:
	
		static void push_location_path(LocationConfig& location, const std::string& line);
		static void push_allowed_methods(LocationConfig& location, const std::string& line);
		static void push_bool(bool& location, const std::string& line, const std::string& name);
		static void push_redirect(LocationConfig& location, const std::string& line);

};

#endif