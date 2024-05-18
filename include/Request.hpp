/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:23:14 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/18 13:16:32 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <sstream>
# include <string>

# include "Log.hpp"

class Request
{
	public:
		Request(Log &logger);
		Request(std::string const &raw, Log &logger);
		~Request();

		enum e_method
		{
			HEAD = 0,
			GET = 1,
			POST = 2,
			PUT = 4,
			PATCH = 8,
			DELETE = 16
		};

		void append(std::string const &str);
		int validate_request_line();
		bool is_valid_method(std::string const &method);
		bool is_allowed_method() const;

	private:
		std::stringstream	raw_;
		std::string			req_line_;
		e_method			method_;
		std::string			url_;
		Log					&log;

		Request(Request const &rhs);
		Request &operator=(Request const &rhs);
};

#endif // REQUEST_HPP
