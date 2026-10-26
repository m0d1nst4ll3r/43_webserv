/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 19:11:20 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/26 20:08:55 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"



void ConfigParser::push_location_path(LocationConfig& location, const std::string& line)
{	
	std::istringstream stream(line);

    std::string directive;
    std::string path;
    std::string end;
    std::string extra;

    if (!(stream >> directive >> path >> end))
        throw std::runtime_error("Error: invalid location path directive.");

    if (directive != "location")
        throw std::runtime_error("Error: invalid location path directive.");

    if (stream >> extra)
        throw std::runtime_error("Error: too many arguments in location path.");

    if (end != "{")
        throw std::runtime_error("Error: missing final {.");

    location.path = path;
}

void ConfigParser::push_allowed_methods(LocationConfig& location, const std::string& line)
{	
	std::istringstream stream(line);

    std::string directive;
    std::string allowedOne;
    std::string allowedTwo;
	std::string allowedThree;
    std::string extra;

    if (!(stream >> directive >> allowedOne))
		throw std::runtime_error("Error: invalid location allowed methods directive.");

    if (directive != "allowed_methods")
		throw std::runtime_error("Error: invalid location allowed methods directive.");
		
	if (!(stream >> allowedTwo))
	{
		if (allowedOne[allowedOne.size() - 1] != ';')
        	throw std::runtime_error("Error: missing final semicolon.");
		
		allowedOne.erase(allowedOne.size() - 1);
		
		if (allowedOne != "GET" && allowedOne != "POST" && allowedOne != "DELETE")
        	throw std::runtime_error("Error: invalid location allowed method.");
		
		location.allowedMethods.push_back(allowedOne);
	}
	else if (!(stream >> allowedThree))
	{
		if (allowedOne != "GET" && allowedOne != "POST" && allowedOne != "DELETE")
        	throw std::runtime_error("Error: invalid location allowed method.");
		
		if (allowedTwo[allowedTwo.size() - 1] != ';')
        	throw std::runtime_error("Error: missing final semicolon.");
		
		allowedTwo.erase(allowedTwo.size() - 1);
		
		if (allowedTwo != "GET" && allowedTwo != "POST" && allowedTwo != "DELETE")
			throw std::runtime_error("Error: invalid location allowed method.");
		
		if (allowedTwo == allowedOne)
			throw std::runtime_error("Error: duplicate allowed methods.");
	
		location.allowedMethods.push_back(allowedOne);
		location.allowedMethods.push_back(allowedTwo);
	}
	else
	{
		if (allowedOne != "GET" && allowedOne != "POST" && allowedOne != "DELETE")
        	throw std::runtime_error("Error: invalid location allowed method.");
		
		if (allowedTwo != "GET" && allowedTwo != "POST" && allowedTwo != "DELETE")
        	throw std::runtime_error("Error: invalid location allowed method.");
		
		if (allowedTwo == allowedOne)
			throw std::runtime_error("Error: duplicate allowed methods.");
		
		if (allowedThree[allowedThree.size() - 1] != ';')
        	throw std::runtime_error("Error: missing final semicolon.");
		
		allowedThree.erase(allowedThree.size() - 1);
		
		if (allowedThree != "GET" && allowedThree != "POST" && allowedThree != "DELETE")
			throw std::runtime_error("Error: invalid location allowed method.");
		
		if (allowedThree == allowedOne || allowedThree == allowedTwo)
			throw std::runtime_error("Error: duplicate allowed methods.");
		
		location.allowedMethods.push_back(allowedOne);
		location.allowedMethods.push_back(allowedTwo);
		location.allowedMethods.push_back(allowedThree);
	}

    if (stream >> extra)
        throw std::runtime_error("Error: too many arguments in location allowed methods.");
}