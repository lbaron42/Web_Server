/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 11:58:31 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/15 19:31:11 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <csignal>
# include <cstddef>
# include <cstring>
# include <sstream>
# include <string>
# include <vector>

# include <fcntl.h>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>

# include "ChunkNorris.hpp"
# include "Headers.hpp"
# include "Log.hpp"
# include "Reply.hpp"
# include "Request.hpp"
# include "Utils.hpp"

class Server;

class CGIHandler{
	
	public:
		CGIHandler(Log &log, int client, size_t max_body_size, Server *owner);
		CGIHandler(const CGIHandler &src);

		bool execute(int pipes[2], Request *request);

		bool read_input();
		bool send_output();
		void on_pipe_close(int fd);
		bool send_reply();
		bool receive();

		~CGIHandler();

	private:
		Log					&log;
		Request				*request;
		int					client_fd;
		int					input;
		int					output;
		pid_t				pid;
		int					status;
		std::vector<char>	wbuf;
		std::vector<char>	rbuf;
		Headers				headers;
		std::vector<char>	reply;
		size_t				max_body_size;
		Server				*owner;
		ChunkNorris			*chunkster;

		void prepare_reply();
		void reply_error();
		void read_headers();

		CGIHandler &operator=(const CGIHandler &src);
};

#endif
