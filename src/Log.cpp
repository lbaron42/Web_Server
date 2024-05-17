/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:58:03 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 05:00:08 by mcutura          ###   ########.fr       */
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
	if (this->verbosity_curr_ < this->verbosity_)
		return *this;
	(*this->out_) << os;
	return *this;
}

Log &Log::operator<<(e_loglevel const &severity)
{
	this->verbosity_curr_ = severity;
	if (this->verbosity_curr_ < this->verbosity_)
		return *this;
	switch (severity) {
		case DEBUG:	(*this->out_) << "[DEBUG] "; break;
		case INFO:	(*this->out_) << "[INFO] "; break;
		case WARN:	(*this->out_) << "[WARN] "; break;
		case ERROR:	(*this->out_) << "[ERROR] "; break;
		default:	(*this->out_) << "[MESSAGE] "; break;
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
