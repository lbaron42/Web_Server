/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 16:21:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/13 08:15:42 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Headers::Headers() : headers()
{}

Headers::~Headers()
{}

Headers::Headers(Headers const &rhs) : headers(rhs.headers)
{}

Headers &Headers::operator=(Headers const &rhs)
{
	if (this == &rhs)	return *this;
	this->headers = rhs.get_headers();
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

bool Headers::is_set(std::string const &key) const
{
	return this->headers.count(key);
}

std::map<std::string, std::string> Headers::get_headers() const
{
	return this->headers;
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

std::vector<char const *> Headers::get_as_env(void) const
{
	std::vector<char const *>	result;
	std::map<std::string, std::string>::const_iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it) {
		std::string tmp(it->first);
		std::transform(
			tmp.begin(),
			tmp.end(),
			tmp.begin(),
			::toupper);
		tmp.append("=" + it->second);
		result.push_back(tmp.c_str());
	}
	return result;
}

void Headers::set_header(std::string const &key, std::string const &value)
{
	if (utils::icompare(key, "set-cookie")) {
		Cookie cookie(value);
		if (!cookie.get_name().empty())
			this->jar.push_back(cookie);
		return;
	}
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

void Headers::set_date(void)
{
	std::time_t	t(std::time(NULL));
	std::string	timestamp(utils::time_tostr(t));
	if (!timestamp.empty())
		this->headers["Date"] = timestamp;
}

bool Headers::has_cookies(void) const
{
	return !this->jar.empty();
}

std::vector<std::string> Headers::get_cookies(void) const
{
	std::vector<std::string>	ret;
	for (std::vector<Cookie>::const_iterator coo = this->jar.begin();
	coo != this->jar.end(); ++coo)
		ret.push_back(coo->to_header());
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
//	Operator overloads
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &os, Headers const &hdrs)
{
	if (hdrs.has_cookies()) {
		std::vector<std::string>	cookies(hdrs.get_cookies());
		for (std::vector<std::string>::const_iterator it = cookies.begin();
		it != cookies.end(); ++it)
			os << *it << "\r\n";
	}
	std::set<std::string> keys = hdrs.get_keys();
	for (std::set<std::string>::const_iterator it = keys.begin();
	it != keys.end(); ++it) {
		os << *it << ": " << hdrs.get_header(*it) << "\r\n";
	}
	return os;
}
