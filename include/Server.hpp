/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/16 17:10:15 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# ifndef STRICT_EVALUATOR
#  define STRICT_EVALUATOR 0
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

namespace marvinX
{
	volatile static sig_atomic_t	g_stopme(0);

	extern "C" void stop_server(int sig);
}

class Server
{
	public:
		Server(std::string const &name, std::string const &port);
		~Server();

		bool initialize();
		void start();

	private:
		static int const	BACKLOG_ = 10;
		std::string const	name_;
		std::string const	port_;
		int					listen_fd_;
		int					epoll_fd_;
		std::set<int>		clients_;

		bool setup_socket();
		void add_client(int listen_fd);
		void close_connection(int fd);
		void handle_request(int fd);
		void cleanup();

		/* no copy */
		Server(Server const &rhs);
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
