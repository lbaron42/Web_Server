/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 04:37:44 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/13 05:59:18 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cookie.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Cookie::Cookie(
			std::string name,
			std::string value,
			std::string expires,
			std::string max_age,
			std::string domain,
			std::string path,
			e_samesite same_site,
			bool secure,
			bool http_only
			)
	:	name(name),
		value(value),
		expires(expires),
		max_age(max_age),
		domain(domain),
		path(path),
		same_site(same_site),
		secure(secure),
		http_only(http_only)
{}

Cookie::Cookie(std::string const &str)
	:	name(),
		value(),
		expires(),
		max_age(),
		domain(),
		path(),
		same_site(LAX),
		secure(false),
		http_only(false)
{
	if (str.empty()) {
		return;
	}
	std::vector<std::string>	crumbs(utils::split(str, ';'));
	std::string::size_type		pos(crumbs[0].find_first_of('='));
	this->name = crumbs[0].substr(0, pos);
	if (pos != std::string::npos)
		this->value = crumbs[0].substr(pos + 1);
	if (!Cookie::is_valid(this->name, this->value)) {
		return;
	}
	for (std::vector<std::string>::const_iterator it = crumbs.begin() + 1;
	it != crumbs.end(); ++it) {
		std::string	key;
		std::string	val;
		pos = it->find_first_of('=');
		key = it->substr(0, pos);
		if (pos != std::string::npos)
			val = it->substr(pos + 1);
		else
			val = std::string();
		if (utils::icompare(key, "httponly"))
			this->http_only = true;
		else if (utils::icompare(key, "secure"))
			this->secure = true;
		else if (utils::icompare(key, "samesite")) {
			if (utils::icompare(val, "strict"))
				this->same_site = STRICT;
			else if (utils::icompare(val, "none"))
				this->same_site = NONE;
			else
				this->same_site = LAX;
		} else if (utils::icompare(key, "domain"))
			this->domain = val;
		else if (utils::icompare(key, "path"))
			this->path = val;
		else if (utils::icompare(key, "expires"))
			this->expires = val;
		else if (utils::icompare(key, "max-age"))
			this->max_age = val;
		// else -> IGNORED
	}
}


Cookie::Cookie(Cookie const &yum)
	:	name(yum.name),
		value(yum.value),
		expires(yum.expires),
		max_age(yum.max_age),
		domain(yum.domain),
		path(yum.path),
		same_site(yum.same_site),
		secure(yum.secure),
		http_only(yum.http_only)
{}

Cookie::~Cookie()
{}

////////////////////////////////////////////////////////////////////////////////
//	Operator overloads
////////////////////////////////////////////////////////////////////////////////

inline bool Cookie::operator==(Cookie const &yum) const
{
	return (
		this->name == yum.name
		&& this->domain == yum.domain
		&& this->path == yum.path
		&& this->secure == yum.secure
	);
}

////////////////////////////////////////////////////////////////////////////////
//	Static methods
////////////////////////////////////////////////////////////////////////////////

bool Cookie::is_valid(std::string const &attr_name,
		std::string const &attr_value)
{
	if (attr_name.empty()
	|| attr_name.find_first_of("()<>@,;:\\\"/[]?={}") != std::string::npos)
		return false;
	for (std::string::const_iterator ch = attr_name.begin();
	ch != attr_name.end(); ++ch)
		if (*ch < 33 || *ch == 127)
			return false;
	for (std::string::const_iterator ch = attr_value.begin();
	ch != attr_value.end(); ++ch)
		if (*ch < 33 || *ch == 127)
			return false;
	std::string tmp(utils::trim(attr_value, "\""));
	if (tmp.find_first_of(",;\\\"") != std::string::npos)
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Getters/Setters
////////////////////////////////////////////////////////////////////////////////

std::string Cookie::get_name(void) const
{
	return this->name;
}

std::string Cookie::get_value(void) const
{
	return this->value;
}

void Cookie::set_value(std::string const &new_value)
{
	this->value = new_value;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

std::string Cookie::to_header(void) const
{
	std::ostringstream	oss;

	oss << "Set-Cookie: " << this->name << "=" << this->value;
	if (!this->expires.empty())
		oss << "; " << "Expires=" << this->expires;
	if (!this->max_age.empty())
		oss << "; " << "Max-Age=" << this->max_age;
	if (!this->domain.empty())
		oss << "; " << "Domain=" << this->domain;
	if (!this->path.empty())
		oss << "; " << "Path=" << this->path;
	if (this->same_site != LAX) {
		oss << "; " << "SameSite="
			<< (this->same_site == STRICT ? "Strict" : "None");
	}
	if (this->secure)
		oss << "; " << "Secure";
	if (this->http_only)
		oss << "; " << "HttpOnly";
	return oss.str();
}
