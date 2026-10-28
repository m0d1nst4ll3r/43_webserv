/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 16:57:44 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/28 19:02:39 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include <stdexcept>

void validate_semicolon(const std::string& line, const std::string& name)
{
	std::size_t semicolon = line.find(';');

	if (semicolon == std::string::npos)
		throw std::runtime_error("Error: " + name + ": missing final semicolon.");

	if (line.find(';', semicolon + 1) != std::string::npos)
		throw std::runtime_error("Error: " + name + ": multiple semicolons.");

	if (semicolon != line.size() - 1)
		throw std::runtime_error("Error: " + name + ": unexpected characters after semicolon.");

	if (semicolon > 0 && (line[semicolon - 1] == ' ' || line[semicolon - 1] == '\t'))
		throw std::runtime_error("Error: " + name + ": whitespace before semicolon.");
}

void checkDuplicate(std::set<std::string>& directives, const std::string& name)
{
	if (!directives.insert(name).second)
		throw std::runtime_error("Error: duplicate " + name + " directive.");
}

void validate_server_config(const Config& config)
{
	if (config.listens.empty())
		throw std::runtime_error("Error: missing listen directive.");

	if (config.root.empty())
		throw std::runtime_error("Error: missing root directive.");
}

void validate_location_config(const LocationConfig& location)
{
	if (location.uploadEnabled && location.uploadStore.empty())
		throw std::runtime_error("Error: upload_enabled is on but upload_store is missing.");

	if (!location.cgiExtension.empty() && location.cgiExecutable.empty())
		throw std::runtime_error("Error: cgi_extension is set but cgi_path is missing.");

	if (location.cgiExtension.empty() && !location.cgiExecutable.empty())
		throw std::runtime_error("Error: cgi_path is set but cgi_extension is missing.");
}