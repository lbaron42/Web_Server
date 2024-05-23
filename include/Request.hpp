/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:23:14 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/23 11:24:21 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <sstream>
# include <string>
# include <vector>

# include "Log.hpp"
# include "Headers.hpp"
# include "Utils.hpp"

class Request
{
	public:
		Request(Log &logger);
		Request(std::string const &raw, Log &logger);
		Request(Request const &rhs);
		~Request();

		enum e_method
		{
			NONE = 0,
			HEAD = 1,
			GET = 2,
			POST = 4,
			PUT = 8,
			PATCH = 16,
			DELETE = 32,
			OPTIONS = 64,
			CONNECT = 128,
			TRACE = 256
		};

		e_method get_method() const;
		std::string get_url() const;
		bool is_version_11() const;
		std::string const get_header(std::string const &key) const;
		Headers get_headers() const;
		std::string get_req_line() const;
		int get_status() const;
		std::string get_target() const;
		bool is_dirlist() const;
		bool is_parsed() const;
		bool is_done() const;

		void set_status(int status);
		void set_target(std::string const &path);
		void set_dirlist(bool value);
		void set_parsed(bool value);

		void append(std::string const &str);
		int validate_request_line();
		bool is_valid_method(std::string const &method);
		bool parse_headers();

	private:
		Log					&log;
		std::stringstream	raw_;
		std::string			req_line;
		e_method			method;
		std::string			url;
		bool				v_11;
		bool				is_parsed_;
		bool				is_dirlist_;
		int					status;
		Headers				headers;
		std::string			target;
		std::vector<char>	payload;

		Request &operator=(Request const &rhs);
};

#endif // REQUEST_HPP
