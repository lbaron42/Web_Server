/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkNorris.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:28:27 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/10 04:15:55 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKNORRIS_HPP
# define CHUNKNORRIS_HPP

# include <cstddef>
# include <sstream>
# include <string>
# include <vector>

# include "Headers.hpp"
# include "Reply.hpp"
# include "Request.hpp"
# include "Utils.hpp"

class ChunkNorris
{
	public:
		ChunkNorris();
		bool is_done() const;
		std::vector<char> *get_unchunked() const;

		/**
		 * Unchunk chunked request message body
		 *
		 * @param request Request* to unchunk
		 * @return false for error, sets request status
		 */
		bool nunchunkMe(Request *request);
		/**
		 * Unchunk chunked received buffer
		 *
		 * @param std::vector<char> &chunk to unchunk
		 * @return false for error
		 */
		bool nunchunkMe(std::vector<char> &chunk);
		~ChunkNorris();

	private:
		std::vector<char>	*nunchunks;
		std::string			remainder;
		size_t				next_size;
		bool				done;

		ChunkNorris(ChunkNorris const &webserv_ranger);
		ChunkNorris &operator=(ChunkNorris const &expendable);
};

#endif // CHUNKNORRIS_HPP
