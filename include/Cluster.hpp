/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/18 19:51:30 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/09 11:11:15 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
# define CLUSTER_HPP

# ifndef STRICT_EVALUATOR
#  define STRICT_EVALUATOR 0
# endif

# ifndef DEBUG_MODE
#  define DEBUG_MODE 0
# endif

# define CONNECTION_TIMEOUT	60
# define CGI_IDLE_TIMEOUT	20

# include <algorithm>
# include <cerrno>
# include <csignal>
# include <ctime>
# include <map>
# include <queue>
# include <set>
# include <vector>

# include "Log.hpp"
# include "Server.hpp"
# include "Request.hpp"
# include "CGIHandler.hpp"

namespace marvinX
{
	volatile static sig_atomic_t	g_stopme(0);

	extern "C" void stop_servers(int sig);
}

class Cluster
{
	public:
		Cluster(Log &log);
		~Cluster();

		void add_server(Server &server);
		bool init_all();
		void start();

	private:
		Log											&log;
		int											epoll_fd;
		std::vector<Server>							servers;
		std::map<ServerData::Address, int>			bound_addresses;
		std::set<std::string>						reserved_ports;
		std::map<int, std::vector<Server const*> >	listen_fds;
		std::map<int, Server const*>				client_fds;
		std::map<int, time_t>						client_timeouts;
		std::queue<std::pair<Request*, int> >		bounce_que;
		std::map<int, CGIHandler*>					cgis;
		std::map<CGIHandler*, Server*>				cgi_hosts;
		std::map<CGIHandler*, time_t>				cgi_timeouts;

		void check_timeouts();
		void manage_bounce_que(void);
		void update_cgi_pipes(void);
		bool manage_cgi(epoll_event const &e);

		Cluster(Cluster const &rhs);
		Cluster &operator=(Cluster const &rhs);
};

#endif // CLUSTER_HPP
