/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:30:06 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/15 10:05:57 by mcutura          ###   ########.fr       */
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
# include <queue>
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
# include "CGIHandler.hpp"
# include "ChunkNorris.hpp"
# include "Utils.hpp"

struct ServerData
{
	ServerData();
	struct Address
	{
		Address();
		std::string							ip;
		std::string							port;
		inline bool operator<(Address const &rhs) const {
			return (
				this->port < rhs.port
				|| (!(rhs.port < this->port) && this->ip < rhs.ip)
				);
		}
		inline bool operator==(Address const &rhs) const {
			return (
				(this->ip == rhs.ip && this->port == rhs.port)
				|| (this->port == rhs.port
					&& (this->ip.compare("0.0.0.0")
					|| rhs.ip.compare("0.0.0.0")))
				);
		}
		~Address();
	};
	struct Location
	{
		Location();
		std::string					location_path;
		std::string					alias;
		std::vector<std::string>	loc_index;
		Request::e_method			allow_methods;
		bool 						autoindex;
		std::string					redirection;
		~Location();
	};

	std::vector<Address>			addresses;
	std::vector<std::string>		hostnames;
	std::map<int, std::string>		error_pages;
	std::vector<std::string>		serv_index;
	std::string						root;
	std::string						logfile;
	size_t							client_max_body_size;
	bool							autoindex;
	Request::e_method				allow_methods;
	std::string						cgi_path;
	std::vector<std::string>		cgi_ext;
	std::vector<Location>			locations;
	~ServerData();
};

template <typename T>
struct DelValue
{
	void operator()(std::pair<int, T*> pair)	{ delete pair.second; }
};

class Server
{
	public:
		Server(ServerData const &server_data, Log &log);
		Server(Server const &rhs);

		const std::vector<ServerData::Address> get_addresses() const;
		const std::vector<std::string> get_hostnames() const;
		std::string get_root() const;
		std::vector<std::pair<int, CGIHandler*> > get_cgi_pipes();
		void set_epoll(int epoll_fd);
		void set_log();

		bool validate_root() const;
		void sort_locations(void);
		int setup_socket(char const *service, char const *node);
		int add_client(int listen_fd);
		void close_connection(int fd);
		bool recv_request(int fd,
				std::queue<std::pair<Request*, int> > &que, CGIHandler **cgi);
		void register_request(int client_fd, Request *request);
		bool handle_request(int fd);
		Server &check_queue(int fd);
		bool matches_hostname(Request *request);
		bool switch_epoll_mode(int fd, uint32_t events);
		bool send_reply(int fd, CGIHandler **cgi);
		void prepare_error_page(Request *request, Headers &hdrs,
				std::vector<char> &payload);
		bool request_timeout(int fd);
		void shutdown_cgi(CGIHandler *cgi);
		std::string translate_uri(std::string const &path_info);
		~Server();

	private:
		ServerData									info;
		Log											&log;
		int											epoll_fd;
		std::set<int>								clients;
		std::map<int, Request*>						requests;
		std::map<int, std::vector<char> >			replies;
		std::set<int>								keep_alive;
		std::map<int, CGIHandler*>					cgis;
		std::map<int, ChunkNorris*>					chunksters;
		std::queue<std::pair<int, CGIHandler*> >	upstream_q;

		bool add_epoll_mode(int fd, uint32_t events);
		void parse_request(int fd);
		Server &drop_request(int fd);
		Server &enqueue_reply(int fd, std::vector<char> const &reply);
		std::string resolve_address(Request *request, Headers &headers);
		void get_head(Request *request, Headers &headers);
		void get_payload(Request *request, Headers &headers,
				std::vector<char> *body);
		bool load_request_body(Request *request);
		void handle_post_request(Request *request, Headers &headers,
				std::vector<char> *body);
		void handle_put_request(Request *request, Headers &headers,
				std::vector<char> *body);
		void handle_delete_request(Request *request, std::vector<char> *body);
		bool is_cgi_request(Request *request, Headers &headers);
		bool handle_cgi(int fd, Request *request);
		void internal_error(int fd, int code);

		/* no assign to object with reference member (here: Log) */
		Server &operator=(Server const &rhs);
};

#endif // SERVER_HPP
