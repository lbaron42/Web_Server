/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/23 11:46:52 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <cstddef>
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
# include <sstream>
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
# include "Headers.hpp"
# include "Utils.hpp"

struct ServerData
{
	struct Address
	{
		std::string	ip;
		std::string	port;
	};

	struct Location
	{
		std::string			alias;
		std::string			index;
		Request::e_method	allowed_methods;
	};

	std::vector<Address>		address;
	std::vector<std::string>	hostname;
	std::string 				root;
	std::string 				index;
	std::vector<Location>		locations;
	size_t						client_max_body_size;
	Request::e_method			allowed_methods;
	bool						directory_listing;
};

class Server
{
	public:
		Server(ServerData const &server_data, Log &log);
		~Server();
		Server(Server const &rhs);

		bool initialize(int epoll_fd);
		std::map<int, Server const*> get_listen_fds() const;
		int add_client(int epoll_fd, int listen_fd);
		void close_connection(int epoll_fd, int fd);
		bool recv_request(int epoll_fd, int fd);
		void handle_request(int fd);
		int send_reply(int epoll_fd, int fd);

	private:
		ServerData							info;
		Log									&log;
		std::set<int>						listen_fds;
		std::set<int>						clients;
		std::map<int, Request*>				requests;
		std::map<int, std::vector<char> >	replies;

		int setup_socket(char const *service, char const *node);
		bool switch_epoll_mode(int epoll_fd, int fd, uint32_t events);
		void parse_request(int fd);
		Server &drop_request(int fd);
		Server &check_queue(int fd);
		Server &enqueue_reply(int fd, std::vector<char> const &reply);
		std::string resolve_address(Request *request);
		void get_head(Request *request, Headers &headers);
		void get_payload(Request *request, Headers &headers,
				std::vector<char> *body);
		int handle_post_request(Request *request, Headers &headers);
		Server &generate_response(Request *request, Headers &headers,
				std::vector<char> const &body, std::vector<char> &repl);

		/* no assign to object with reference member (here: Log) */
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
