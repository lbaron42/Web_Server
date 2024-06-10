/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkNorris.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:28:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/10 03:57:55 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkNorris.hpp"

// ChunkNorris need big memory, new nunchunks to chop 'em all
ChunkNorris::ChunkNorris() : remainder(), next_size(0), done(false)
{
	this->nunchunks = new (std::nothrow) std::vector<char>;
}

bool ChunkNorris::is_done() const
{
	return this->done;
}

std::vector<char> *ChunkNorris::get_unchunked() const
{
	return this->nunchunks;
}

bool ChunkNorris::nunchunkMe(Request *request)
{
	if (!this->nunchunks) {
		request->set_status(500);
		return false;
	}
	std::stringstream	ss(request->load_body());
	size_t				size(0);
	for (std::string tmp; std::getline(ss, tmp);) {
		if (tmp[tmp.size() - 1] != '\r') {
			this->remainder = tmp;
			this->next_size = size;
			return true;
		}
		if (!size) {
			if (!utils::str_tohex(tmp, &size)) {
				request->set_status(400);
				return false;
			}
			if (!size) {
				this->done = true;
				return true;
			}
			continue;
		}
		if (tmp.size() > size + 1) {
			request->set_status(400);
			return false;
		}
		if (tmp.size() < size + 1) {
			this->remainder = tmp;
			this->next_size = size;
			return true;
		}
		this->nunchunks->insert(
			this->nunchunks->end(),
			tmp.begin(),
			tmp.end() - 1);
		size = 0;
	}
	return true;
}

bool ChunkNorris::nunchunkMe(std::vector<char> &chunk)
{
	if (!this->nunchunks)
		return false;
	size_t	size(0);
	if (this->next_size) {
		if (chunk.size() >= this->next_size - this->remainder.size()) {
			this->nunchunks->insert(
				this->nunchunks->end(),
				this->remainder.begin(),
				this->remainder.end());
			this->nunchunks->insert(
				this->nunchunks->end(),
				chunk.begin(),
				chunk.begin() + this->next_size - this->remainder.size());
			this->remainder.clear();
			this->next_size = 0;
			chunk.erase(
				chunk.begin(),
				chunk.begin() + this->next_size - this->remainder.size());
		} else {
			this->remainder.append(chunk.begin(),chunk.end());
			return true;
		}
	} else if (!this->remainder.empty()) {
		size_t	i(0);
		while (i < chunk.size() && chunk[i] != '\n')
			++i;
		if (i == chunk.size()) {
			this->remainder.append(chunk.begin(), chunk.end());
			return true;
		}
		this->remainder.append(chunk.begin(), chunk.begin() + i - 1);
		if (!utils::str_tohex(this->remainder, &size))
			return false;
		this->remainder.clear();
		chunk.erase(chunk.begin(), chunk.begin() + i + 1);
	}
	while (!chunk.empty()) {
		if (!size){
			while (chunk[0] == '\r' || chunk[0] == '\n')
				chunk.erase(chunk.begin());
			std::vector<char>::iterator it(
				std::find(chunk.begin(), chunk.end(), '\n'));
			if (it == chunk.end()) {
				this->remainder.append(chunk.begin(), chunk.end());
				return true;
			}
			std::string	tmp(chunk.begin(), it);
			if (!utils::str_tohex(tmp, &size))
				return false;
			if (!size) {
				this->done = true;
				return true;
			}
			chunk.erase(chunk.begin(), it);
			continue;
		}
		if (chunk.size() < size + 2) {
			this->remainder.append(chunk.begin(), chunk.end());
			this->next_size = size;
			return true;
		}
		this->nunchunks->insert(
			this->nunchunks->end(),
			chunk.begin(),
			chunk.begin() + size);
		chunk.erase(chunk.begin(), chunk.begin() + size);
		size = 0;
	}
	return true;
}

ChunkNorris::~ChunkNorris()
{
	if (this->nunchunks)
		delete this->nunchunks;
}
