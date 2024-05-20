/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 16:21:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 01:44:43 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Headers::Headers()
{}

Headers::~Headers()
{}

Headers::Headers(Headers const &rhs) : headers(rhs.headers)
{}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

bool Headers::is_set(std::string const &key) const
{
	return this->headers.count(key);
}

std::string Headers::get_header(std::string const &key) const
{
	std::map<std::string, std::string>::const_iterator it;
	it = this->headers.find(key);
	if (it != this->headers.end())
		return it->second;
	return std::string();
}

std::string getKey(std::pair<std::string, std::string> p) { return p.first; }

std::set<std::string> Headers::get_keys() const
{
	std::set<std::string> keys;
	std::transform(this->headers.begin(), this->headers.end(),
		std::inserter(keys, keys.end()), getKey);
	return keys;
}

void Headers::set_header(std::string const &key, std::string const &value)
{
	std::map<std::string, std::string>::iterator it;
	it = this->headers.find(key);
	if (it != this->headers.end())
		this->headers[it->first] = value;
	else
		this->headers.insert(std::make_pair(key, value));
}

void Headers::unset_header(std::string const &key)
{
	if (this->headers.count(key))
		this->headers.erase(key);
}

////////////////////////////////////////////////////////////////////////////////
//	Operator overloads
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &os, Headers const &hdrs)
{
	std::set<std::string> keys = hdrs.get_keys();
	for (std::set<std::string>::const_iterator it = keys.begin();
	it != keys.end(); ++it) {
		os << *it << ": " << hdrs.get_header(*it) << std::endl;
	}
	return os;
}
