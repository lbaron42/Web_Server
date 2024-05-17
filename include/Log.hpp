/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:51:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 01:03:42 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_HPP
# define LOG_HPP

# include <ctime>
# include <fstream>
# include <iostream>
#include <ostream>

enum e_loglevel
{
	DEBUG,
	INFO,
	WARN,
	ERROR
};

class Log
{
	public:
		Log();
		virtual ~Log();

		void set_output(std::ostream *stream, bool is_file);
		template <typename T>
		Log &operator<<(T const &obj)
		{
			this->timestamp();
			(*this->out_) << obj;
			return *this;
		}
		Log &operator<<(std::ostream &(*os)(std::ostream &));

	private:
		std::ostream	*out_;
		bool			is_file_;

		void timestamp();

		/* no copy */
		Log(Log const &rhs);
		Log &operator=(Log const &rhs);
};

#endif // LOG_HPP
