/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:58:03 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 02:46:36 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include <iostream>

Log::Log() : out_(&std::cout), is_file_(false)
{}

Log::~Log()
{
	this->set_output(0, false);
}

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

Log &Log::operator<<(std::ostream &(*os)(std::ostream &))
{
	(*this->out_) << os;
	return *this;
}

void Log::timestamp()
{
	char		timestamp[20];
	std::time_t	t;

	t = std::time(NULL);
	if (std::strftime(timestamp, sizeof(timestamp), \
		"%Y-%m-%d %H:%M:%S", std::localtime(&t)))
		(*this->out_) << "[" << timestamp << "]\t";
}
