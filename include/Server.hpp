/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/14 11:05:15 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

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

class Server
{
	public:
		Server();
		~Server();

		bool initialize();
		void start();

	private:
		static int const	BACKLOG_ = 10;
		std::string			port_;
		int					listen_fd_;
		int					epoll_fd_;
		std::set<int>		clients_;

		bool setup_socket();
		void add_client(int listen_fd);
		void close_connection(int fd);
		void handle_request(int fd);
};

#endif // SERVER_HPP
