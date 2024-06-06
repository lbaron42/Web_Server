/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 04:37:44 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/06 15:43:28 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cookie.hpp"
#include "Utils.hpp"

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
	std::string tmp(trim(attr_value, "\""));
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

// TODO
// Cookie CookieJar::get_cookie(
// 			std::string const &name,
// 			std::string const &domain,
// 			std::string const &path,
// 			bool secure) const
// {
// }

bool CookieJar::set_cookie(Cookie const &biscuit)
{
	if (!Cookie::is_valid(biscuit.get_name(), biscuit.get_value()))
		return false;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////
