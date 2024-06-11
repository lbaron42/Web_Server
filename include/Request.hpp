/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:23:14 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/12 00:19:00 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <algorithm>
# include <cstddef>
# include <ios>
# include <iterator>
# include <sstream>
# include <string>
# include <vector>

# include "Log.hpp"
# include "Headers.hpp"
# include "Utils.hpp"

class Request
{
	public:
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
		
		static e_method parse_methods(std::string const &str);

		e_method get_method() const;
		std::string get_method_as_str() const;
		std::string get_url() const;
		std::string get_query() const;
		bool is_version_11() const;
		std::string const get_header(std::string const &key) const;
		Headers get_headers() const;
		std::string get_req_line() const;
		int get_status() const;
		std::string get_target() const;
		std::string get_script() const;
		std::string get_path() const;
		std::vector<char> get_payload() const;
		std::vector<std::string> get_filenames() const;
		bool is_dirlist() const;
		bool is_parsed() const;
		bool is_done() const;
		size_t get_loaded_body_size() const;
		bool is_body_loaded() const;
		bool is_bounced() const;
		bool is_chunked() const;

		void set_status(int status);
		void set_target(std::string const &path);
		void set_script(std::string const &script);
		void set_path(std::string const &path);
		void set_dirlist(bool value);
		void set_parsed(bool value);
		void set_bounced(bool value);

		void append(std::string const &str);
		bool validate_url();
		int validate_request_line();
		bool is_valid_method(std::string const &method);
		bool parse_headers();
		std::string load_body();
		bool load_payload(size_t size);
		void drop_payload();
		bool load_multipart(std::string const &boundary, size_t max_body_size);

	private:
		Log							&log;
		std::stringstream			raw_;
		std::string					req_line;
		e_method					method;
		std::string					url;
		std::string					query;
		bool						v_11;
		bool						is_parsed_;
		bool						is_dirlist_;
		int							status;
		Headers						headers;
		std::string					target;
		std::string					script;
		std::string					path;
		size_t						loaded_body_size;
		bool						is_body_loaded_;
		std::vector<char>			payload;
		std::vector<std::string>	filenames;
		bool						bounced;
		bool						is_chunked_;

		Request &operator=(Request const &rhs);
};

#endif // REQUEST_HPP
