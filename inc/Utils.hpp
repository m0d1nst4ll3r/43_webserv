/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 16:58:14 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/27 19:31:56 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <set>

void	validate_semicolon(const std::string& line, const std::string& name);
void	checkDuplicate(std::set<std::string>& directives, const std::string& name);

#endif