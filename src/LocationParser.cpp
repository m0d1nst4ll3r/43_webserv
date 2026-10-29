/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 19:11:20 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/28 19:02:13 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Utils.hpp"

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
	if (!location.allowedMethods.empty())
	throw std::runtime_error("Error: duplicate allowed_methods directive.");
	
	std::istringstream stream(line);

	std::string directive;
	std::string allowed;
	std::vector<std::string> methods;
	bool endFound = false;

	if (!(stream >> directive))
		throw std::runtime_error("Error: invalid location allowed methods directive.");

	if (directive != "allowed_methods")
		throw std::runtime_error("Error: invalid location allowed methods directive.");

	while (stream >> allowed)
	{
		if (endFound)
			throw std::runtime_error("Error: unexpected argument after semicolon.");

		if (!allowed.empty() && allowed[allowed.size() - 1] == ';')
		{
			allowed.erase(allowed.size() - 1);
			endFound = true;
		}
		
		if (allowed != "GET" && allowed != "POST" && allowed != "DELETE")
			throw std::runtime_error("Error: invalid location allowed method.");
		
		if (std::find(methods.begin(), methods.end(), allowed) != methods.end())
			throw std::runtime_error("Error: duplicate allowed methods.");

		methods.push_back(allowed);
	}
	
	if (!endFound)
		throw std::runtime_error("Error: missing final semicolon.");

	if (methods.size() > 3)
		throw std::runtime_error("Error: too many arguments in location allowed methods.");

	location.allowedMethods = methods;
}

void ConfigParser::push_bool(bool& location, const std::string& line, const std::string& name)
{
	validate_semicolon(line, name);

	std::istringstream stream(line);
	
	std::string directive;
	std::string set;
	std::string	extra;

	if (!(stream >> directive >> set))
		throw std::runtime_error("Error: invalid location " + name + " directive.");

	if (directive != name)
		throw std::runtime_error("Error: invalid location " + name + " directive.");

	if (stream >> extra)
		throw std::runtime_error("Error: too many arguments in location " + name + ".");
	
	if (set == "on;")
		location = true;
	else if (set == "off;")
		location = false;
	else
		throw std::runtime_error("Error: wrong argument in location " + name + ".");	
}

void ConfigParser::push_redirect(LocationConfig& location, const std::string& line)
{
	validate_semicolon(line, "redirect");

	std::istringstream stream(line);
	
	std::string directive;
	std::string code;
	std::string url;
	std::string	extra;
	int	codeConversion;

	if (!(stream >> directive >> code >> url))
		throw std::runtime_error("Error: invalid location redirect directive.");

	if (directive != "return")
		throw std::runtime_error("Error: invalid location redirect directive.");

	if (stream >> extra)
		throw std::runtime_error("Error: too many arguments in location redirect.");

	for (std::size_t i = 0; i < code.size(); i++)
	{
		if (!std::isdigit(static_cast<unsigned char>(code[i])))
			throw std::runtime_error("Error: invalid redirect status code.");
	}
		
	std::istringstream codeStream(code);
		
	if (!(codeStream >> codeConversion) || (codeConversion != 301 && codeConversion != 302 && codeConversion != 303 && codeConversion != 307 && codeConversion != 308))
		throw std::runtime_error("Error: invalid redirect status code.");

	if (location.redirectCode != 0 || !location.redirect.empty())
		throw std::runtime_error("Error: duplicate redirect directive.");

	url.erase(url.size() - 1);
	
	location.redirectCode = codeConversion;
	location.redirect = url;
}