/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 07:56:07 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/02 09:39:27 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLY_HPP
# define REPLY_HPP

# include <cstddef>
# include <ctime>
# include <fstream>
# include <iomanip>
# include <ios>
# include <iterator>
# include <sstream>
# include <string>
# include <vector>

# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>

# include "Request.hpp"
# include "Headers.hpp"
# include "Utils.hpp"

class Reply
{
	public:
		static std::string const get_status_line(bool v11, int status);
		static std::string const get_status_message(int status);
		static std::string const get_content(std::string const &filename);
		static std::vector<char> const get_payload(
				std::string const &filename);
		static std::string const get_listing(std::string const &filename,
				std::string const &url);
		static std::string const generate_error_page(int status = 500);
		static size_t get_html_size(int status);
		static size_t get_html_size(std::string const &listed_directory,
				std::string const &url);
		static void generate_response(Request *request, Headers &headers,
		std::vector<char> const &body, std::vector<char> &repl);

	private:
		Reply();
		~Reply();
		Reply(Reply const &rhs);
		Reply &operator=(Reply const &rhs);
};

#endif // REPLY_HPP
