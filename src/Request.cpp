/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:22:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 20:46:46 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Request::Request(Log &logger) : log(logger)
{}

Request::Request(std::string const &raw, Log &logger) : raw_(raw), log(logger)
{}

Request::~Request()
{}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Request::append(std::string const &str)
{
	this->raw_ << str;
}

int Request::validate_request_line()
{
	if (std::getline(this->raw_, this->req_line_, '\n'))
	{
		log << Log::DEBUG << "Request line: " << this->req_line_ << std::endl;

		std::string::size_type	first(this->req_line_.find(' '));
		if (first == std::string::npos)
			return 400;
		if (!this->is_valid_method(this->req_line_.substr(0, first)))
			return 400;
		if (!this->is_allowed_method())
			return 405;
		log << Log::DEBUG << "Method:		|"
			<< this->req_line_.substr(0, first)
			<< "|" << std::endl;

		std::string::size_type	second(this->req_line_.find(' ', first + 1));
		if (second == std::string::npos)
			return 400;
		this->url_ = this->req_line_.substr(first + 1, second - first - 1);
		log << Log::DEBUG << "URL:		|" << this->url_ << "|" << std::endl;

		std::string::size_type	end(this->req_line_.find('\r', second + 1));
		log << Log::DEBUG << "Version:	|"
			<< this->req_line_.substr(second + 1, end - second - 1)
			<< "|" << std::endl;
		if (this->req_line_.substr(second + 1, end - second - 1) == "HTTP/1.1")
			return 200;
	}
	return 0;
}

bool Request::is_valid_method(std::string const &method)
{
	if (method == "GET") {
		this->method_ = Request::GET;
		return true;
	} else if (method == "POST") {
		this->method_ = Request::POST;
		return true;
	} else if (method == "DELETE") {
		this->method_ = Request::DELETE;
		return true;
	}
	return false;
}

bool Request::is_allowed_method() const
{
	return true;
}

