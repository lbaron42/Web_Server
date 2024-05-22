/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:23:14 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 21:55:48 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <sstream>
# include <string>
# include <vector>

# include "Log.hpp"
# include "Headers.hpp"

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

		e_method get_method() const;
		std::string get_url() const;
		bool is_version_11() const;
		std::string const get_header(std::string const &key) const;
		Headers get_headers() const;
		std::string get_req_line() const;

		void append(std::string const &str);
		int validate_request_line();
		bool is_valid_method(std::string const &method);
		bool parse_headers();

	private:
		std::stringstream	raw_;
		Log					&log;
		std::string			req_line;
		e_method			method;
		std::string			url;
		bool				v_11;
		Headers				headers;
		std::vector<char>	payload;

		Request(Request const &rhs);
		Request &operator=(Request const &rhs);
};

#endif // REQUEST_HPP
