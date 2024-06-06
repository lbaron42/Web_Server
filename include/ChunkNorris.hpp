/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkNorris.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:28:27 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/05 14:02:09 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKNORRIS_HPP
# define CHUNKNORRIS_HPP

# include <string>
# include <vector>

# include "Request.hpp"
# include "Headers.hpp"
# include "Reply.hpp"

class ChunkNorris
{
	public:
		ChunkNorris();
		bool is_done() const;
		static void chunkMe(std::vector<char> &body, Headers &headers);
		bool nunchunkMe(Request *request);
		~ChunkNorris();

	private:
		std::vector<char>	*nunchunks;
		bool				done;

		ChunkNorris(ChunkNorris const &webserv_ranger);
		ChunkNorris &operator=(ChunkNorris const &expendable);
};

#endif // CHUNKNORRIS_HPP