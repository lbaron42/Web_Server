/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:22:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/23 11:24:03 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <ios>

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Request::Request(Log &logger)
	:	log(logger),
		raw_(std::string(),
			std::ios_base::in | std::ios_base::out | std::ios_base::ate),
		method(Request::NONE),
		v_11(true),
		is_parsed_(false),
		is_dirlist_(false),
		status(0),
		headers(),
		target(),
		payload()
{}

Request::Request(std::string const &raw, Log &logger)
	:	log(logger),
		raw_(raw, std::ios_base::in | std::ios_base::out | std::ios_base::ate),
		method(Request::NONE),
		v_11(true),
		is_parsed_(false),
		is_dirlist_(false),
		status(0),
		headers(),
		target(),
		payload()
{}

Request::Request(Request const &rhs)
	:	log(rhs.log),
		raw_(rhs.raw_.str(),
			std::ios_base::in | std::ios_base::out | std::ios_base::ate),
		method(Request::NONE),
		v_11(true),
		is_parsed_(false),
		is_dirlist_(false),
		status(0),
		headers(),
		target(),
		payload()
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

int Request::get_status() const
{
	return this->status;
}

std::string Request::get_target() const
{
	return this->target;
}

bool Request::is_dirlist() const
{
	return this->is_dirlist_;
}

bool Request::is_parsed() const
{
	return this->is_parsed_;
}

bool Request::is_done() const
{
	return (this->raw_.rdbuf()->in_avail() == 0);
}

void Request::set_status(int status)
{
	this->status = status;
}

void Request::set_target(std::string const &path)
{
	this->target = path;
}

void Request::set_dirlist(bool value)
{
	this->is_dirlist_ = value;
}

void Request::set_parsed(bool value)
{
	this->is_parsed_ = value;
	log << Log::DEBUG << "Remaining raw buffer:" << std::endl
		<< this->raw_.str() << std::endl;
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
	if (size_of_stream(this->raw_) < 16)
		return 0;
	while (std::getline(this->raw_, this->req_line)) {
		if (this->req_line.empty()
		|| (this->req_line.length() == 1 && this->req_line == "\r"))
			continue;
		trim(this->req_line);
		log << Log::DEBUG << "Request line: " << this->req_line << std::endl;

		std::string::size_type first(this->req_line.find(' '));
		if (first == std::string::npos)
			return 400;
		if (first < 3 || first > 6
		|| !this->is_valid_method(this->req_line.substr(0, first)))
			return 400;
		log << Log::DEBUG << "Method:		["
			<< this->req_line.substr(0, first) << "]"
			<< std::endl;

		std::string::size_type second(this->req_line.find(' ', first + 1));
		if (second == std::string::npos || second == first + 1)
			return 400;
		this->url = this->req_line.substr(first + 1, second - first - 1);
		log << Log::DEBUG << "URL:		[" << this->url << "]" << std::endl;

		std::string::size_type end(this->req_line.find('\r', second + 1));
		if (end == std::string::npos) {
			log << Log::WARN
				<< "Request line terminated by '\\n' instead of '\\r\\n'"
				<< std::endl;
		} else if (end != second + 9 || end != this->req_line.length() - 1)
			return 400;
		log << Log::DEBUG << "Version:	["
			<< this->req_line.substr(second + 1, end - second - 1)
			<< "]" << std::endl;
		std::string vers(this->req_line.substr(second + 1, end - second - 1));
		trim(vers, "\r");
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
	static std::string const methodnames[] = { "NONE",
											"HEAD",
											"GET",
											"POST",
											"PUT",
											"PATCH",
											"DELETE",
											"OPTIONS",
											"CONNECT",
											"TRACE" };

	for (size_t i = 1; i < sizeof(methodnames); ++i) {
		if (method.c_str() == methodnames[i]) {
			this->method = static_cast<Request::e_method>(1 << --i);
			return true;
		}
	}
	return false;
}

bool Request::parse_headers()
{
	std::string	header;

	while (std::getline(this->raw_, header)) {
		if (this->raw_.eof()) {
			if (header.empty())
				return true;
			this->raw_.clear();
			this->raw_.str(std::string());
			this->raw_ << header;
			log << Log::DEBUG << "Partial header returned to raw stream"
				<< std::endl;
			return false;
		}
		if (header.empty() || header == "\r") {
			std::string tmp;
			std::stringstream remainder(std::string(),
				std::ios_base::in | std::ios_base::out | std::ios_base::ate);
			while (std::getline(this->raw_, tmp))
				remainder << tmp;
			this->raw_.clear();
			this->raw_.str(std::string());
			this->raw_ << remainder.rdbuf();
			return true;
		}
		std::string::size_type div = header.find(':');
		if (div == std::string::npos) {
			log << Log::WARN << "Received malformatted header" << std::endl;
			continue;
		}
		std::string key = header.substr(0, div);
		std::string val = header.substr(div + 1);
		this->headers.set_header(trim(key), trim(val, " \t\r"));
	}
	return false;
}
