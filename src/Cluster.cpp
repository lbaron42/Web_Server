/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/18 19:51:33 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/27 00:16:33 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Signal handling
////////////////////////////////////////////////////////////////////////////////

namespace marvinX {
	extern "C" void stop_servers(int sig)
	{
		(void)sig;
		g_stopme = 1;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Cluster::Cluster(Log &log) : log(log)
{}

Cluster::~Cluster()
{}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Cluster::add_server(Server &server)
{
	this->servers.push_back(server);
}

bool Cluster::init_all()
{
	this->epoll_fd = STRICT_EVALUATOR	? epoll_create(42)
										: epoll_create1(EPOLL_CLOEXEC);
	if (this->epoll_fd == -1) {
		log << Log::ERROR << "Failed to create epoll file descriptor"
			<< std::endl;
		return false;
	}

	for (std::vector<Server>::iterator it = this->servers.begin();
	it != this->servers.end(); ++it) {
		if (!it->initialize(this->epoll_fd))
			return false;
		std::map<int, Server const*> tmp = it->get_listen_fds();
		this->listen_fds.insert(tmp.begin(), tmp.end());
	}
	return true;
}

void Cluster::start()
{
	int const	timeout = 420;
	int const	max_events = 42;
	epoll_event	events[max_events];

	typedef std::map<int, Server const*>::const_iterator FdMapCIterator;

	while (!marvinX::g_stopme) {
		int n_events = epoll_wait(this->epoll_fd, events, max_events, timeout);
		if (n_events == -1) {
			if (!marvinX::g_stopme)
				log << Log::ERROR << "Failed epoll_wait" << std::endl;
			break;
		}
		for (int i = 0; i < n_events; ++i) {
			int fd = events[i].data.fd;
			FdMapCIterator it = this->listen_fds.find(fd);
			if (it != this->listen_fds.end()) {
				int client = const_cast<Server*>(it->second)
								->add_client(this->epoll_fd, it->first);
				if (client != -1)
					this->client_fds.insert(std::make_pair(client, it->second));
				continue;
			}
			it = this->client_fds.find(fd);
			if (it == this->client_fds.end()) {
				log << Log::WARN << "Untracked file descriptor in epoll"
					<< std::endl;
				continue;
			}
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
				const_cast<Server*>(it->second)
					->close_connection(this->epoll_fd, it->first);
				this->client_fds.erase(it->first);
				continue;
			}
			if (events[i].events & EPOLLIN) {
				if (const_cast<Server*>(it->second)
					->recv_request(this->epoll_fd, it->first)) {
					this->client_fds.erase(it->first);
				}
			}
			if (events[i].events & EPOLLOUT) {
				if (const_cast<Server*>(it->second)
					->send_reply(this->epoll_fd, it->first) == -1) {
					this->client_fds.erase(it->first);
				}
			}
		}
	}
	if (marvinX::g_stopme)
		log << Log::INFO << "SIGINT received - shutting down"
			<< std::endl;
}
