/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/27 00:59:40 by mcutura          ###   ########.fr       */
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
# include <cstddef>
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
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>

# include "Log.hpp"
# include "Reply.hpp"
# include "Request.hpp"
# include "Headers.hpp"
# include "Utils.hpp"

struct ServerData
{
	ServerData();
	struct Address
	{
		std::string								ip;
		std::string								port;
	};
	struct Location
	{
		std::string								location_path;
		std::string								alias;
		std::vector<std::string>				loc_index;
		Request::e_method						allow_methods;
		bool 									is_redirection;
	};

	std::vector<Address>						addresses;
	std::vector<std::string>					hostnames;
	std::map<int, std::string>					error_pages;
	std::vector<std::string>					serv_index;
	std::string									root;
	size_t										client_max_body_size;
	bool										autoindex;
	Request::e_method							allow_methods;
	std::vector<Location>						locations;
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
		bool handle_request(int fd);
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
		void handle_post_request(Request *request, Headers &headers);
		Server &generate_response(Request *request, Headers &headers,
				std::vector<char> const &body, std::vector<char> &repl);

		/* no assign to object with reference member (here: Log) */
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
