/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:58:03 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/27 12:03:27 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Log::Log() : out_(&std::cout), is_file_(false), \
	verbosity_(Log::INFO), verbosity_curr_(Log::INFO)
{}

Log::~Log()
{
	this->set_output(0, false);
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Log::set_output(std::ostream *stream, bool is_file)
{
	if (this->is_file_)
	{
		this->out_->flush();
		delete this->out_;
	}
	this->out_ = stream;
	this->is_file_ = is_file;
}

void Log::set_verbosity(e_loglevel const &verbosity)
{
	this->verbosity_ = verbosity;
}

Log &Log::operator<<(std::ostream &(*os)(std::ostream &))
{
	if (!this->out_ || this->verbosity_curr_ < this->verbosity_)
		return *this;
	(*this->out_) << os;
	return *this;
}

Log &Log::operator<<(e_loglevel const &severity)
{
	this->verbosity_curr_ = severity;
	if (!this->out_ || this->verbosity_curr_ < this->verbosity_)
		return *this;
	switch (severity) {
		case DEBUG: (*this->out_) << CYAN << "[DEBUG]    " << RESET; break;
		case INFO:  (*this->out_) << GREEN << "[INFO]    " << RESET; break;
		case WARN:  (*this->out_) << YELLOW << "[WARN]    " << RESET; break;
		case ERROR: (*this->out_) << RED << "[ERROR]    " << RESET; break;
		default:    (*this->out_) << MAGENTA << "[MESSAGE]    " << RESET; break;
	}
	this->timestamp();
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

void Log::timestamp()
{
	char		timestamp[20];
	std::time_t	t;

	t = std::time(NULL);
	if (std::strftime(timestamp, sizeof(timestamp), \
		"%Y-%m-%d %H:%M:%S", std::localtime(&t)))
		(*this->out_) << "[" << timestamp << "]\t";
}
