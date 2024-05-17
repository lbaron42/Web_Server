/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 23:51:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 04:24:13 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_HPP
# define LOG_HPP

# include <ctime>
# include <fstream>
# include <iostream>
#include <ostream>

class Log
{
	public:
		enum e_loglevel
		{
			DEBUG,
			INFO,
			WARN,
			ERROR
		};

		Log();
		virtual ~Log();

		void set_output(std::ostream *stream, bool is_file);
		void set_verbosity(e_loglevel const &verbosity);
		template <typename T>
		Log &operator<<(T const &obj)
		{
			if (this->verbosity_curr_ < this->verbosity_)
				return *this;
			(*this->out_) << obj;
			return *this;
		}
		Log &operator<<(e_loglevel const &level);
		Log &operator<<(std::ostream &(*os)(std::ostream &));

	private:
		std::ostream	*out_;
		bool			is_file_;
		e_loglevel		verbosity_;
		e_loglevel		verbosity_curr_;

		void timestamp();

		/* no copy */
		Log(Log const &rhs);
		Log &operator=(Log const &rhs);
};

#endif // LOG_HPP
