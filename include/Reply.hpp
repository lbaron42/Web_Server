/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 07:56:07 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 08:19:23 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLY_HPP
# define REPLY_HPP

# include <sstream>
# include <string>

class Reply
{
	public:
		Reply();
		~Reply();

		static std::string const get_status_line(int status);
		static std::string const get_status_message(int status);

	private:
		Reply(Reply const &rhs);
		Reply &operator=(Reply const &rhs);
};

#endif // REPLY_HPP
