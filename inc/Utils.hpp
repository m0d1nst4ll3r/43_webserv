/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 16:58:14 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/28 18:43:51 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <set>
#include "Config.hpp"

void	validate_semicolon(const std::string& line, const std::string& name);
void	checkDuplicate(std::set<std::string>& directives, const std::string& name);
void	validate_server_config(const Config& config);
void	validate_location_config(const LocationConfig& location);

#endif