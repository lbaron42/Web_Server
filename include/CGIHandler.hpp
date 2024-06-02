/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 11:58:31 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/02 12:31:54 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <cstddef>
# include <cstring>
# include <map>
# include <sstream>
# include <string>
# include <vector>

# include <fcntl.h>
# include <sys/epoll.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>

# include "Log.hpp"
# include "Headers.hpp"
# include "Request.hpp"
# include "Reply.hpp"

class CGIHandler{
	
	public:
		CGIHandler(Log &log, int client, size_t max_body_size);
		CGIHandler(const CGIHandler &src);

		// how to pass status of exit?
		bool execute(int pipes[2], Request *request);

		bool read_input();
		bool send_output();
		bool send_reply();
		bool receive();

		~CGIHandler();

	/*
		When is child process done? how we get notified?
	*/

	private:
		Log									&log;
		Request								*request;
		int									client_fd;
		int									input;
		int									output;
		pid_t								pid;
		int									status;
		std::vector<char>					buff;
		Headers								headers;
		std::vector<char>					reply;
		size_t								max_body_size;

		ssize_t read_from_pipe(std::vector<char> &buff);
		ssize_t write_to_pipe(std::vector<char> &buff);

		CGIHandler &operator=(const CGIHandler &src);
};

#endif
