/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:58:03 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/28 23:31:37 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Log::Log()
	:	out_(&std::cout),
		is_file_(false),
		verbosity_(Log::INFO),
		verbosity_curr_(Log::INFO)
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

	bool use_colors = !this->is_file_;

	switch (severity) {
		case DEBUG: (*this->out_) << (use_colors ? CYAN : "")
			<< "[DEBUG]	" << (use_colors ? RESET : ""); break;
		case INFO: (*this->out_) << (use_colors ? GREEN : "")
			<< "[INFO]	" << (use_colors ? RESET : ""); break;
		case WARN: (*this->out_) << (use_colors ? YELLOW : "")
			<< "[WARN]	" << (use_colors ? RESET : ""); break;
		case ERROR: (*this->out_) << (use_colors ? RED : "")
			<< "[ERROR]	" << (use_colors ? RESET : ""); break;
		default: (*this->out_) << (use_colors ? MAGENTA : "")
			<< "[MESSAGE]	" << (use_colors ? RESET : ""); break;
	}
	this->timestamp();
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

void Log::timestamp()
{
	std::time_t	t;
	char		timestamp[32];

	t = std::time(NULL);
	if (std::strftime(timestamp, sizeof(timestamp), \
		"%Y-%m-%d %H:%M:%S", std::localtime(&t)))
		(*this->out_) << "[" << timestamp << "]\t";
}
