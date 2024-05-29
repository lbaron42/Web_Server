/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/18 19:51:30 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/29 07:32:46 by mcutura          ###   ########.fr       */
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

# include <algorithm>
# include <csignal>
# include <map>
# include <queue>
# include <vector>

# include "Log.hpp"
# include "Server.hpp"
# include "Request.hpp"

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
		std::map<int, std::vector<Server const*> >	listen_fds;
		std::map<int, Server const*>				client_fds;
		std::queue<std::pair<Request*, int> >		bounce_que;

		Cluster(Cluster const &rhs);
		Cluster &operator=(Cluster const &rhs);
};

#endif // CLUSTER_HPP
