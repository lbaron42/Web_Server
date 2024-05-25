/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 16:23:16 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/25 14:51:32 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
# define HEADERS_HPP

# include <algorithm>
# include <map>
# include <iostream>
# include <set>
# include <string>

class Headers
{
	public:
		Headers();
		~Headers();
		Headers(Headers const &rhs);
		Headers &operator=(Headers const &rhs);

		bool is_set(std::string const &key) const;
		std::map<std::string, std::string> get_headers() const;
		std::string get_header(std::string const &key) const;
		std::set<std::string> get_keys() const;
		void set_header(std::string const &key, std::string const &value);
		void unset_header(std::string const &key);

	private:
		std::map<std::string, std::string>	headers;
};

std::ostream &operator<<(std::ostream &os, Headers const &hdrs);

#endif // HEADERS_HPP
