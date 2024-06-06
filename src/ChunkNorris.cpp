/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkNorris.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:28:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/05 14:26:47 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkNorris.hpp"

// ChunkNorris need big memory, new nunchunks to chop 'em all
ChunkNorris::ChunkNorris() : done(false)
{
	this->nunchunks = new (std::nothrow) std::vector<char>;
}

bool ChunkNorris::is_done() const
{
	return this->done;
}

// TODO
void ChunkNorris::chunkMe(std::vector<char> &body, Headers &headers)
{
	(void)body;
	(void)headers;
}

/*
 * Return false:
 *		- if allocation failed -> reply 500 Internal Server Error
 *		- if bad chunk size -> request->status is already 400
 * Check is_done() to verify unchunking completed
 */
bool ChunkNorris::nunchunkMe(Request *request)
{
	if (!this->nunchunks
	|| !request->load_chunk())
		return false;
	std::vector<char> tmp(request->get_payload());
	if (!tmp.size()) {
		this->done = true;
		return true;
	}
	this->nunchunks->insert(
		this->nunchunks->end(),
		tmp.begin(),
		tmp.end());
	request->drop_payload();
	return true;
}

ChunkNorris::~ChunkNorris()
{
	if (this->nunchunks)
		delete this->nunchunks;
}