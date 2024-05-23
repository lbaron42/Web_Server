/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 07:56:07 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 22:08:11 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLY_HPP
# define REPLY_HPP

# include <cstddef>
# include <fstream>
# include <ios>
# include <iterator>
# include <sstream>
# include <string>
# include <vector>

# include "Utils.hpp"

class Reply
{
	public:
		static std::string const get_status_line(bool v11, int status);
		static std::string const get_status_message(int status);
		static std::string const get_content(std::string const &filename);
		static std::vector<char> const get_payload(
				std::string const &filename);
		static std::string const get_listing(std::string const &filename);

	private:
		Reply();
		~Reply();
		Reply(Reply const &rhs);
		Reply &operator=(Reply const &rhs);
};

#endif // REPLY_HPP
