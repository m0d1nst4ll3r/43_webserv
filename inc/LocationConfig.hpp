/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 18:45:53 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/23 16:33:29 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig
{
	public:
		
		LocationConfig();
		~LocationConfig();
		
		std::string path;
		std::vector<std::string> allowedMethods;

		std::string redirect;
		std::string root;

		bool autoindex;
		std::string index;

		bool uploadEnabled;
		std::string uploadStore;

		std::string cgiExtension;
		std::string cgiExecutable;
};

#endif
