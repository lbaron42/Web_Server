/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/18 19:51:33 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/15 09:50:12 by mcutura          ###   ########.fr       */
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

Cluster::Cluster(Log &log)
	:	log(log),
		epoll_fd(-1),
		servers(),
		bound_addresses(),
		reserved_ports(),
		listen_fds(),
		client_fds(),
		client_timeouts(),
		bounce_que(),
		cgis(),
		cgi_hosts(),
		cgi_timeouts()
{}

Cluster::~Cluster()
{
	while (!this->bounce_que.empty()) {
		delete bounce_que.front().first;
		bounce_que.pop();
	}
	std::map<int, std::vector<Server const*> >::iterator it;
	for (it = this->listen_fds.begin(); it != this->listen_fds.end(); ++it)
		close(it->first);
	std::map<int, Server const*>::iterator cl;
	for (cl = this->client_fds.begin(); cl != this->client_fds.end(); ++cl)
		close(cl->first);
	this->servers.clear();
	close(this->epoll_fd);
}

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
		if (!it->validate_root()) {
			log << Log::ERROR << "Misconfigured root for server: "
				<< (it->get_hostnames().empty()	? "unnamed"
												: it->get_hostnames()[0])
				<< std::endl;
			return false;
		}
		it->sort_locations();
		it->set_log();
		it->set_epoll(this->epoll_fd);
		std::vector<ServerData::Address>	addresses(it->get_addresses());
		std::vector<ServerData::Address>::const_iterator ad = addresses.begin();
		for ( ; ad != addresses.end(); ++ad) {
			std::map<ServerData::Address, int>::const_iterator bound;
			bound = this->bound_addresses.find(*ad);
			if (bound == this->bound_addresses.end()) {
				if (this->reserved_ports.count(ad->port)) {
					log << Log::ERROR << "Unable to start server: "
						<< (it->get_hostnames().empty()	?
							"unnamed"	:	it->get_hostnames()[0])
						<< " at: " << ad->ip << ":" << ad->port
						<< "	=> Port reserved" << std::endl;
					return false;
				}
				if (!ad->ip.compare("0.0.0.0")) {
					std::map<ServerData::Address, int>::const_iterator	bound;
					for (bound = this->bound_addresses.begin();
					bound != this->bound_addresses.end(); ++bound) {
						if (*ad == bound->first) {
							log << Log::ERROR << "Server: "
								<< (it->get_hostnames().empty()	?
									"unnamed"	:	it->get_hostnames()[0])
								<< " => Unable to reserve port "
								<< ad->port << std::endl;
							return false;
						}
					}
				}
				int sfd = it->setup_socket(ad->port.c_str(), \
						ad->ip.empty() ? NULL : ad->ip.c_str());
				if (sfd == -1) {
					log << Log::ERROR << "Failed to setup listening socket for "
						<< (it->get_hostnames().empty()	?
								"unnamed" : it->get_hostnames()[0]) << " at "
						<< ad->ip << ":" << ad->port << std::endl;
					return false;
				}
				this->bound_addresses.insert(std::make_pair(*ad, sfd));
				this->listen_fds.insert(std::make_pair(
					sfd, std::vector<Server const*>(1, &(*it))));

				epoll_event event = {};
				event.events = EPOLLIN;
				event.data.fd = sfd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &event)) {
					log << Log::ERROR << "Failed to add fd " << sfd
						<< " to epoll_ctl" << std::endl;
					return false;
				}
				log << Log::DEBUG << "Added fd " << sfd
					<< " to epoll_ctl" << std::endl;
				if (!ad->ip.compare("0.0.0.0"))
					this->reserved_ports.insert(ad->port);
			} else {
				std::map<int, std::vector<Server const*> >::iterator lfd;
				lfd = this->listen_fds.find(bound->second);
				if (lfd == this->listen_fds.end()) {
					log << Log::ERROR << "Address - socket binding mismatch"
						<< std::endl;
					return false;
				}
				log << Log::DEBUG << "Listening socket address already bound"
					<< std::endl;
				lfd->second.push_back(&(*it));
			}
		}
		log << Log::INFO << "Initialized server: "
			<< (it->get_hostnames().empty()	? "unnamed"
											: it->get_hostnames()[0])
			<< std::endl;
	}
	return true;
}

void Cluster::start()
{
	int const	timeout = 420;
	int const	max_events = 42;
	epoll_event	events[max_events];

	while (!marvinX::g_stopme) {
		errno = 0;
		int n_events = epoll_wait(
			this->epoll_fd,
			events,
			max_events,
			timeout);
		if (n_events == -1) {
			if (!marvinX::g_stopme) {
				if (errno == EINTR)
					continue;
				log << Log::ERROR << "Failed epoll_wait " << errno << std::endl;
			}
			break;
		}
		this->manage_bounce_que();
		this->update_cgi_pipes();
		this->check_timeouts();
		for (int i = 0; i < n_events; ++i) {
			int fd = events[i].data.fd;
			std::map<int, std::vector<Server const*> >::iterator listenfd;
			listenfd = this->listen_fds.find(fd);
			if (listenfd != this->listen_fds.end()) {
				int client = const_cast<Server*>(listenfd->second.front())
								->add_client(listenfd->first);
				if (client != -1) {
					this->client_fds.insert(std::make_pair(
						client, listenfd->second.front()));
					this->client_timeouts.insert(
						std::make_pair(client, std::time(NULL)));
				}
				continue;
			}
			std::map<int, Server const*>::iterator it;
			it = this->client_fds.find(fd);
			if (it == this->client_fds.end()) {
				if (!this->manage_cgi(events[i])) {
					log << Log::WARN << "Untracked file descriptor in epoll: "
						<< fd << std::endl;
					if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL)) {
						log << Log::ERROR << "Failed epoll_del fd: " << fd
							<< " errno: " << errno << std::endl;
					}
				}
				continue;
			}
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
				const_cast<Server*>(it->second)->close_connection(it->first);
				this->client_timeouts.erase(it->first);
				this->client_fds.erase(it->first);
				continue;
			}
			CGIHandler	*cgi = NULL;
			if (events[i].events & EPOLLIN) {
				if (!const_cast<Server*>(it->second)
				->recv_request(it->first, this->bounce_que, &cgi)) {
					if (cgi) {
						this->cgi_hosts[cgi]->close_connection(it->first);
						std::map<int, CGIHandler*>::iterator cgh;
						for (cgh = this->cgis.begin();
						cgh != this->cgis.end();) {
							if (cgh->second == cgi) {
								std::map<int, CGIHandler*>::iterator tmp(cgh++);
								this->cgis.erase(tmp);
							} else
								++cgh;
						}
						this->cgi_hosts.erase(cgi);
						this->cgi_timeouts.erase(cgi);
					}
					this->client_timeouts.erase(it->first);
					this->client_fds.erase(it->first);
				} else {
					this->client_timeouts[it->first] = std::time(NULL);
					if (cgi)
						this->cgi_timeouts[cgi] = std::time(NULL);
				}
			} else if (events[i].events & EPOLLOUT) {
				if (!const_cast<Server*>(it->second)
				->send_reply(it->first, &cgi)) {
					if (cgi) {
						std::map<int, CGIHandler*>::iterator cgh;
						for (cgh = this->cgis.begin();
						cgh != this->cgis.end();) {
							if (cgh->second == cgi) {
								std::map<int, CGIHandler*>::iterator tmp(cgh++);
								this->cgis.erase(tmp);
							} else
								++cgh;
						}
						this->cgi_hosts[cgi]->close_connection(it->first);
						this->cgi_hosts.erase(cgi);
						this->cgi_timeouts.erase(cgi);
					}
					this->client_timeouts.erase(it->first);
					this->client_fds.erase(it->first);
				} else {
					this->client_timeouts[it->first] = std::time(NULL);
					if (cgi)
						this->cgi_timeouts[cgi] = std::time(NULL);
				}
			}
		}
	}
	if (marvinX::g_stopme)
		log << Log::INFO << "SIGINT received - shutting down"
			<< std::endl;
}

void Cluster::check_timeouts()
{
	time_t	now;
	std::time(&now);
	for (
		std::map<int, time_t>::iterator it = this->client_timeouts.begin();
		it != this->client_timeouts.end();
		) {
		if (std::difftime(now, it->second) > CONNECTION_TIMEOUT) {
			log << Log::DEBUG << "Client " << it->first
				<< " connection timed out" << std::endl;
			if (!const_cast<Server*>(client_fds[it->first])
				->request_timeout(it->first)) {
				this->client_fds.erase(it->first);
			}
			std::map<int, time_t>::iterator dead(it++);
			this->client_timeouts.erase(dead);
		} else
			++it;
	}
	std::time(&now);
	std::map<CGIHandler*, time_t>::iterator cgih;
	for (
		cgih = this->cgi_timeouts.begin();
		cgih != this->cgi_timeouts.end();
		) {
		if (std::difftime(now, cgih->second) > CGI_IDLE_TIMEOUT) {
			std::map<CGIHandler*, Server*>::iterator it;
			it = this->cgi_hosts.find(cgih->first);
			if (it != this->cgi_hosts.end()) {
				it->second->shutdown_cgi(cgih->first);
				this->cgi_hosts.erase(it);
			}
			std::map<CGIHandler*, time_t>::iterator dead(cgih++);
			this->cgi_timeouts.erase(dead);
		} else
			++cgih;
	}
}

void Cluster::manage_bounce_que(void)
{
	while (!this->bounce_que.empty()) {
		std::pair<Request *, int> bounced = this->bounce_que.front();
		this->bounce_que.pop();
		Request	*request = bounced.first;
		int		cfd = bounced.second;
		std::vector<Server>::iterator sr = this->servers.begin();
		for ( ; sr != this->servers.end(); ++sr) {
			if (sr->matches_hostname(request)) {
				log << Log::DEBUG << "Found matching host: "
					<< request->get_header("host") << std::endl;
				this->client_fds[cfd] = &(*sr);
				sr->register_request(cfd, request);
				if (sr->handle_request(cfd)
				&& !sr->switch_epoll_mode(cfd, EPOLLOUT)) {
					this->client_fds.erase(cfd);
				}
				break;
			}
		}
		if (sr == this->servers.end()) {
			log << Log::DEBUG << "No server found matching hostname: "
				<< request->get_header("host")
				<< std::endl;
			const_cast<Server*>(this->client_fds[cfd])
				->register_request(cfd, request);
			if (const_cast<Server*>(this->client_fds[cfd])
			->handle_request(cfd)
			&& !(const_cast<Server*>(this->client_fds[cfd])
			->switch_epoll_mode(cfd, EPOLLOUT)))
				this->client_fds.erase(cfd);
		}
	}
}

void Cluster::update_cgi_pipes(void)
{
	for (std::vector<Server>::iterator it = this->servers.begin();
	it != this->servers.end(); ++it) {
		std::vector<std::pair<int, CGIHandler*> >	tmp(it->get_cgi_pipes());
		if (tmp.empty())	continue;
		std::vector<std::pair<int, CGIHandler*> >::iterator cg;
		for (cg = tmp.begin(); cg != tmp.end(); ++cg) {
			this->cgis[cg->first] = cg->second;
			this->cgi_hosts[cg->second] = &*it;
			this->cgi_timeouts[cg->second] = std::time(NULL);
		}
	}
	return ;
}

bool Cluster::manage_cgi(epoll_event const &e)
{
	std::map<int, CGIHandler*>::iterator cg(this->cgis.find(e.data.fd));
	if (cg == this->cgis.end())
		return false;
	if (e.events & EPOLLIN) {
		log << Log::DEBUG << "CGI Read pipe ready" << std::endl;
		this->cgi_timeouts[cg->second] = std::time(NULL);
		if (!cg->second->read_input()) {
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, e.data.fd, NULL)) {
				log << Log::ERROR << "Failed epoll_del fd: "
					<< e.data.fd << std::endl;
			}
			close(e.data.fd);
			this->cgis.erase(e.data.fd);
		}
	} else if (e.events & EPOLLOUT) {
		log << Log::DEBUG << "CGI Write pipe ready" << std::endl;
		this->cgi_timeouts[cg->second] = std::time(NULL);
		if (!cg->second->send_output()) {
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, e.data.fd, NULL)) {
				log << Log::ERROR << "Failed epoll_del fd: "
					<< e.data.fd << std::endl;
			}
			close(e.data.fd);
			this->cgis.erase(e.data.fd);
		}
	} else if (e.events & EPOLLERR
	|| e.events & EPOLLHUP) {
		cg->second->on_pipe_close(e.data.fd);
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, e.data.fd, NULL))
			log << Log::ERROR << "Fail epoll_del HUP " << e.data.fd << std::endl;
		this->cgis.erase(e.data.fd);
		log << Log::DEBUG << "CGI pipe closed" << std::endl;
	}
	return true;
}
