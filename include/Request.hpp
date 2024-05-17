/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:23:14 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 09:59:57 by mcutura          ###   ########.fr       */
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
			GET,
			POST,
			DELETE
		};

		void append(std::string const &str);
		int validate_request_line();
		bool is_allowed_method(std::string const &method);

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
