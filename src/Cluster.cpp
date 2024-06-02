/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/18 19:51:33 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/02 10:35:28 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"
#include <csignal>

////////////////////////////////////////////////////////////////////////////////
//	Signal handling
////////////////////////////////////////////////////////////////////////////////

namespace marvinX {
	extern "C" void stop_servers(int sig)
	{
		g_stopme = 1;
		(void)sig;
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
		listen_fds(),
		client_fds(),
		bounce_que()
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
		it->sort_locations();
		it->set_epoll(this->epoll_fd);
		std::vector<ServerData::Address>	addresses(it->get_addresses());
		std::vector<ServerData::Address>::const_iterator ad = addresses.begin();
		for ( ; ad != addresses.end(); ++ad) {
			std::map<ServerData::Address, int>::const_iterator bound;
			bound = this->bound_addresses.find(*ad);
			if (bound == this->bound_addresses.end()) {
				int sfd = it->setup_socket(ad->port.c_str(), \
						ad->ip.empty() ? NULL : ad->ip.c_str());
				if (sfd == -1) {
					log << Log::ERROR << "Failed to setup listening socket for "
						<< it->get_hostnames()[0] << " at "
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
					continue;
				}
				log << Log::DEBUG << "Added fd " << sfd
					<< " to epoll_ctl" << std::endl;
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
			<< it->get_hostnames()[0] << std::endl;
	}
	return true;
}

// TODO: implement connection timeouts
void Cluster::start()
{
	int const	timeout = 420;
	int const	max_events = 42;
	epoll_event	events[max_events];

	while (!marvinX::g_stopme) {
		int n_events = epoll_wait(this->epoll_fd, events, max_events, timeout);
		if (n_events == -1) {
			if (!marvinX::g_stopme)
				log << Log::ERROR << "Failed epoll_wait" << std::endl;
			break;
		}
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
				}
				continue;
			}
			// TODO
			std::map<int, CGIHandler*>::iterator cg(this->cgis.find(fd));
			if (cg != this->cgis.end()) {
				if (events[i].events & EPOLLERR
				|| events[i].events & EPOLLHUP) {
					close(fd);
					this->cgis.erase(fd);
					log << Log::DEBUG << "CGI pipe closed" << std::endl;
					continue;
				}
				if (events[i].events & EPOLLIN) {
					cg->second->read_input();
				}
				if (events[i].events & EPOLLOUT) {
					cg->second->send_output();
				}
				continue;
			}
			std::map<int, Server const*>::iterator it;
			it = this->client_fds.find(fd);
			if (it == this->client_fds.end()) {
				log << Log::WARN << "Untracked file descriptor in epoll"
					<< std::endl;
				continue;
			}
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
				const_cast<Server*>(it->second)->close_connection(it->first);
				this->client_fds.erase(it->first);
				continue;
			}
			if (events[i].events & EPOLLIN) {
				if (const_cast<Server*>(it->second)
				->recv_request(it->first, this->bounce_que)) {
					this->client_fds.erase(it->first);
				}
			}
			if (events[i].events & EPOLLOUT) {
				if (!const_cast<Server*>(it->second)
				->send_reply(it->first)) {
					this->client_fds.erase(it->first);
				}
			}
		}
		this->manage_bounce_que();
		this->update_cgi_pipes();
	}
	if (marvinX::g_stopme)
		log << Log::INFO << "SIGINT received - shutting down"
			<< std::endl;
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
				log << Log::DEBUG << "Found matching host" << std::endl;
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
			log << Log::DEBUG << "No server matching hostname found"
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
		}
	}
	return ;
}
