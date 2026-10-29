/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltourbe <ltourbe@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:33:42 by ltourbe           #+#    #+#             */
/*   Updated: 2026/10/27 18:02:34 by ltourbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
	: redirectCode(0), redirect(""), autoindex(false), uploadEnabled(false)
{
}

LocationConfig::~LocationConfig()
{
}

