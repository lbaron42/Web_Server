/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/19 14:44:56 by mcutura          ###   ########.fr       */
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

# include <algorithm>
# include <cstdlib>
# include <map>
# include <set>
# include <string>
# include <vector>

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

struct ServerData
{
	struct Address
	{
		std::string	ip;
		std::string	port;

		Address(std::string const &ip, std::string const &port)
			: ip(ip), port(port) {}
		~Address() {}
		Address(Address const &rhs) : ip(rhs.ip), port(rhs.port) {}
		Address &operator=(Address const &rhs)
		{
			if (this == &rhs)	return *this;
			this->ip = rhs.ip;
			this->port = rhs.port;
			return *this;
		}
	};

	std::vector<Address>		address;
	std::vector<std::string>	hostname;
	std::string 				root;
	std::string 				index;

	ServerData(std::string const &root, std::string const &index)
		: root(root), index(index) {}
	~ServerData() {}
	ServerData(ServerData const &rhs) : address(rhs.address), \
		hostname(rhs.hostname), root(rhs.root), index(rhs.index) {}
	ServerData &operator=(ServerData const &rhs)
	{
		if (this == &rhs)	return *this;
		this->address = rhs.address;
		this->hostname = rhs.hostname;
		return *this;
	}
};

class Server
{
	public:
		Server(ServerData const &server_data, Log &log);
		~Server();
		Server(Server const &rhs);

		bool initialize(int epoll_fd);
		std::map<int, Server const*> *get_listen_fds() const;
		int add_client(int epoll_fd, int listen_fd);
		void close_connection(int epoll_fd, int fd);
		int recv_request(int epoll_fd, int fd);
		int send_reply(int epoll_fd, int fd);

	private:
		ServerData					info;
		Log							&log;
		std::set<int>				listen_fds;
		std::set<int>				clients;
		std::map<int, Request*>		requests;
		std::map<int, std::string>	replies;

		int setup_socket(char const *service, char const *node);

		/* cannot assign to object with reference member (here: Log) */
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
