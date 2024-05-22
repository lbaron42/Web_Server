/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:22:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 03:03:41 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Request::Request(Log &logger) : log(logger), v_11(true)
{}

Request::Request(std::string const &raw, Log &logger) : raw_(raw), log(logger)
{}

Request::~Request()
{}

////////////////////////////////////////////////////////////////////////////////
//	Getters/Setters
////////////////////////////////////////////////////////////////////////////////

Request::e_method Request::get_method() const
{
	return this->method;
}

std::string Request::get_url() const
{
	return this->url;
}

bool Request::is_version_11() const
{
	return this->v_11;
}

std::string const Request::get_header(std::string const &key) const
{
	return this->headers.get_header(key);
}

Headers Request::get_headers() const
{
	return this->headers;
}

std::string Request::get_req_line() const
{
	return this->req_line;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Request::append(std::string const &str)
{
	this->raw_ << str;
}

int Request::validate_request_line()
{
	while (std::getline(this->raw_, this->req_line)) {
		if (this->req_line.empty()
		|| (this->req_line.size() == 1 && this->req_line == "\r"))
			continue;
		log << Log::DEBUG << "Request line: " << this->req_line << std::endl;

		std::string::size_type first(this->req_line.find(' '));
		if (first == std::string::npos)
			return 400;
		if (!this->is_valid_method(this->req_line.substr(0, first)))
			return 400;
		log << Log::DEBUG << "Method:		|"
			<< this->req_line.substr(0, first) << "|"
			<< std::endl;

		std::string::size_type second(this->req_line.find(' ', first + 1));
		if (second == std::string::npos)
			return 400;
		this->url = this->req_line.substr(first + 1, second - first - 1);
		log << Log::DEBUG << "URL:		|" << this->url << "|" << std::endl;

		std::string::size_type end(this->req_line.find('\r', second + 1));
		if (end == std::string::npos) {
			end = this->req_line.size();
			log << Log::WARN
				<< "Request line terminated by '\\n' instead of '\\r\\n'"
				<< std::endl;
		} else {
			this->req_line.erase(end);
		}
		log << Log::DEBUG << "Version:	|"
			<< this->req_line.substr(second + 1, end - second - 1)
			<< "|" << std::endl;
		std::string vers(this->req_line.substr(second + 1, end - second - 1));
		if (vers == "HTTP/1.1") {
			this->v_11 = true;
			return 200;
		} else if (vers == "HTTP/1.0") {
			this->v_11 = false;
			return 200;
		} else
			return 505;
		break;
	}
	return 400;
}

bool Request::is_valid_method(std::string const &method)
{
	static std::string const methodnames[] = { "HEAD",
											"GET",
											"POST",
											"PUT",
											"PATCH",
											"DELETE" };

	for (size_t i = 0; i < sizeof(methodnames); ++i) {
		if (method.c_str() == methodnames[i]) {
			if (i--)	this->method = static_cast<Request::e_method>(1 << i);
			return true;
		}
	}
	return false;
}

bool Request::parse_headers()
{
	std::string	header;

	while (std::getline(this->raw_, header)) {
		if (header.empty() || (header.size() == 1 && header == "\r"))
			return true;
		std::string::size_type div = header.find(':');
		if (div == std::string::npos) {
			log << Log::WARN << "Invalid header in request" << std::endl;
			continue;
		}
		std::string key = header.substr(0, div);
		std::string val = header.substr(div + 1);
		if (val[val.size() - 1] == '\r')
			val.erase(val.size() - 1);
		this->headers.set_header(key, val);
	}
	return false;
}
