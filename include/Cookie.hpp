/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 04:19:27 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/05 10:27:15 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COOKIE_HPP
# define COOKIE_HPP

# include <ctime>
# include <sstream>
# include <string>

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
		Cookie &operator=(Cookie const &yum);

		static bool is_valid(std::string const &attr_name,
				std::string const &attr_value);

		std::string set_cookie(void) const;
		void bake_cookie(time_t new_expiration);
		void eat_cookie(void);

		~Cookie();

	private:
		std::string	name;
		std::string	value;
		std::string	expires;
		std::string	max_age;
		std::string	domain;
		std::string	path;
		e_samesite	same_site;
		bool		secure;
		bool		http_only;

};

#endif // COOKIE_HPP
