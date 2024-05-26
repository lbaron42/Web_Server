/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:22:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/26 22:43:07 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <vector>

namespace {
	static char const *const	methodnames[] = {
		"NONE", "HEAD", "GET", "POST", "PUT", "PATCH",
	"DELETE", "OPTIONS", "CONNECT", "TRACE" };
	static size_t const			n_methods = 10;
}

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Request::Request(std::string const &raw, Log &logger)
	:	log(logger),
		raw_(raw, std::ios_base::in | std::ios_base::out | std::ios_base::ate),
		req_line(),
		method(Request::NONE),
		url(),
		query(),
		v_11(true),
		is_parsed_(false),
		is_dirlist_(false),
		status(0),
		headers(),
		target(),
		loaded_body_size(0),
		is_body_loaded_(false),
		payload()
{}

Request::Request(Request const &rhs)
	:	log(rhs.log),
		raw_(rhs.raw_.str(),
			std::ios_base::in | std::ios_base::out | std::ios_base::ate),
		req_line(rhs.req_line),
		method(rhs.method),
		url(rhs.url),
		query(rhs.query),
		v_11(rhs.v_11),
		is_parsed_(rhs.is_parsed_),
		is_dirlist_(rhs.is_dirlist_),
		status(rhs.status),
		headers(rhs.headers),
		target(rhs.target),
		loaded_body_size(rhs.loaded_body_size),
		is_body_loaded_(rhs.is_body_loaded_),
		payload(rhs.payload)
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

std::string Request::get_query() const
{
	return this->query;
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

std::vector<char> Request::get_payload() const
{
	return this->payload;
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

size_t Request::get_loaded_body_size() const
{
	return this->loaded_body_size;
}

bool Request::is_body_loaded() const
{
	return this->is_body_loaded_;
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
//	Static methods
////////////////////////////////////////////////////////////////////////////////

Request::e_method Request::parse_methods(std::string const &str)
{
	int					allowed_methods = 0;
	std::string			method;
	std::stringstream	ss(str);

	while (ss >> method) {
		for (size_t i = 1; i < n_methods; ++i) {
			if (!method.compare(methodnames[i])) {
				allowed_methods |= (1 << --i);
				break;
			}
		}
	}
	return static_cast<Request::e_method>(allowed_methods);
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
	if (size_of_stream(this->raw_) < 15)
		return 0;
	while (std::getline(this->raw_, this->req_line)) {
		if (this->req_line.empty()
		|| (this->req_line.length() == 1 && this->req_line == "\r"))
			continue;
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
		std::string::size_type q(this->url.find('?'));
		if (q != std::string::npos && q) {
			this->query = this->url.substr(q);
			this->url.erase(q);
		}
		log << Log::DEBUG << "URL:		[" << this->url << "]" << std::endl;
		if (!this->query.empty()) {
			log << Log::DEBUG << "Query		[" << this->query << "]"
				<< std::endl;
		}

		std::string::size_type end(this->req_line.find('\r', second + 1));
		if (end == std::string::npos) {
			log << Log::WARN
				<< "Request line terminated by '\\n' instead of '\\r\\n'"
				<< std::endl;
		} else if (end != second + 9 || end != this->req_line.length() - 1)
			return 400;
		this->req_line = trim(this->req_line, "\r");
		log << Log::DEBUG << "Version:	["
			<< this->req_line.substr(second + 1, end - second - 1)
			<< "]" << std::endl;
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
	for (size_t i = 1; i < n_methods; ++i) {
		if (!method.compare(methodnames[i])) {
			this->method = static_cast<Request::e_method>(1 << --i);
			return true;
		}
	}
	log << Log::DEBUG << method << " is not a valid method" << std::endl;
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

void Request::load_payload(size_t size)
{
	std::istreambuf_iterator<char>	body_start(this->raw_);
	std::istreambuf_iterator<char>	body_end(body_start);

	std::streampos	begin = this->raw_.tellg();
	std::streampos	end = this->raw_.tellp();

	bool	partial(static_cast<std::streampos>(size) > (end - begin));

	log << Log::DEBUG << "Stream begin: " << begin
		<< " | Stream end: " << end << std::endl;
	if (partial) {
		log << Log::DEBUG << "Partial body in buffer: "
			<< (end - begin) << "/" << size << " bytes received"
			<< std::endl;
		std::advance(body_end, end - begin);
	} else {
		std::advance(body_end, size);
	}
	this->payload.reserve(size);
	this->payload.insert(this->payload.begin(), body_start, body_end);
	this->loaded_body_size = partial ? static_cast<size_t>(end - begin) : size;
	this->is_body_loaded_ = !partial;
	log << Log::DEBUG << "AFTER Stream begin: " << this->raw_.tellg()
		<< " | Stream end: " << this->raw_.tellp() << std::endl;
}
