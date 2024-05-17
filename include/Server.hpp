/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 10:09:40 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# ifndef STRICT_EVALUATOR
#  define STRICT_EVALUATOR 0
# endif

# ifndef DEBUG_MODE
#  define DEBUG_MODE 0
# endif

# include <csignal>
# include <cstdlib>
# include <fstream>
# include <iostream>
# include <map>
# include <set>
# include <string>

# include <arpa/inet.h>
# include <fcntl.h>
# include <netdb.h>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>

# include "Log.hpp"
# include "Reply.hpp"
# include "Request.hpp"

namespace marvinX
{
	volatile static sig_atomic_t	g_stopme(0);

	extern "C" void stop_server(int sig);
}

class Server
{
	public:
		Server(std::string const &name, std::string const &port, Log &logger);
		~Server();

		bool initialize();
		void start();

	private:
		int							listen_fd_;
		int							epoll_fd_;
		std::string const			name_;
		std::string const			port_;
		std::string const			root_;
		std::set<int>				clients_;
		std::map<int, Request*>		requests_;
		std::map<int, std::string>	replies_;
		Log							&log;

		bool setup_socket();
		void add_client(int listen_fd);
		void close_connection(int fd);
		void recv_request(int fd);
		void send_reply(int fd);
		void cleanup();

		/* no copy */
		Server(Server const &rhs);
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
