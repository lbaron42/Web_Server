/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 04:19:27 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/08 21:38:37 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COOKIE_HPP
# define COOKIE_HPP

# include <ctime>
# include <map>
# include <sstream>
# include <string>

# include "Utils.hpp"

class Cookie
{
	public:
		enum e_samesite {
			NONE,
			STRICT,
			LAX
		};

		Cookie(
			std::string name,
			std::string value,
			std::string expires = std::string(),
			std::string max_age = std::string(),
			std::string domain = std::string(),
			std::string path = std::string(),
			e_samesite same_site = LAX,
			bool secure = false,
			bool http_only = false
			);
		Cookie(Cookie const &yum);

		inline bool operator==(Cookie const &yum) const;

		static bool is_valid(std::string const &attr_name,
				std::string const &attr_value);

		std::string get_name(void) const;
		std::string get_value(void) const;
		void set_value(std::string const &new_value);
		std::string to_header(void) const;

		~Cookie();

	private:
		std::string const	name;
		std::string			value;
		std::string			expires;
		std::string			max_age;
		std::string			domain;
		std::string			path;
		e_samesite			same_site;
		bool				secure;
		bool				http_only;

		Cookie &operator=(Cookie const &yum);
};

class CookieJar
{
	public:
		CookieJar();
		CookieJar(CookieJar const &rhs);
		CookieJar &operator=(CookieJar const &rhs);

		Cookie get_cookie(
			std::string const &name,
			std::string const &domain = std::string(),
			std::string const &path = std::string(),
			bool secure = false) const;
		bool set_cookie(Cookie const &biscuit);

		~CookieJar();

	private:
		std::map<std::string, Cookie>	jar;
};

#endif // COOKIE_HPP
