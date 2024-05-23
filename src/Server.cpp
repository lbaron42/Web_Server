/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 21:29:08 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Signal handling
////////////////////////////////////////////////////////////////////////////////
namespace marvinX {
	extern "C" void stop_server(int sig)
	{
		(void)sig;
		g_stopme = 1;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////
Server::Server(std::string const &name, std::string const &port, Log &logger) \
	: name_(name), port_(port), root_("/tmp"), log(logger)
{}

struct del_value
{
	void operator()(std::pair<int, Request*> pair)	{ delete pair.second; }
};

Server::~Server()
{
	std::for_each(this->requests_.begin(), this->requests_.end(), del_value());
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

bool Server::initialize()
{
	if (!setup_socket())
		return false;
	if (STRICT_EVALUATOR)
	{
		if ((this->epoll_fd_ = epoll_create(42)) == -1) {
			log << Log::ERROR << "Failed to create epoll file descriptor"
				<< std::endl;
			this->cleanup();
			return false;
		}
		fcntl(this->epoll_fd_, F_SETFL, O_CLOEXEC);
	}
	else
	{
		if ((this->epoll_fd_ = epoll_create1(EPOLL_CLOEXEC)) == -1) {
			log << Log::ERROR << "Failed to create epoll file descriptor"
				<< std::endl;
			this->cleanup();
			return false;
		}
	}

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = listen_fd_;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event)) {
		log << Log::ERROR << "Failed to add fd to epoll_ctl" << std::endl;
		this->cleanup();
		return false;
	}
	log << Log::INFO << "Server initialized: "
		<< this->name_ << ":" << this->port_ << std::endl;
	return true;
}

void Server::start()
{
	int const	timeout = 420;
	int const	max_events = 42;
	epoll_event	events[max_events];

	while (!marvinX::g_stopme) {
		int n_events = epoll_wait(this->epoll_fd_, events, max_events, timeout);
		if (n_events == -1) {
			if (!marvinX::g_stopme)
				log << Log::ERROR << "Failed epoll_wait" << std::endl;
			break;
		}
		for (int i = 0; i < n_events; ++i) {
			int fd = events[i].data.fd;
			if (this->listen_fd_ == fd) {
				this->add_client(fd);
				continue;
			}
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
				this->close_connection(fd);
				continue;
			}
			if (events[i].events & EPOLLIN) {
				this->recv_request(fd);
			}
			if (events[i].events & EPOLLOUT) {
				this->send_reply(fd);
			}
		}
	}
	this->cleanup();
	log << Log::INFO << "SIGINT received - server shutting down" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

bool Server::setup_socket()
{
	int			sfd(-1);
	addrinfo	*servinfo;
	addrinfo	*ptr;
	addrinfo	hints = {};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (int ret = getaddrinfo(NULL, port_.c_str(), &hints, &servinfo)) {
		log << Log::ERROR << "getaddrinfo error: " \
			<< gai_strerror(ret) << std::endl;
		return false;
	}
	for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
		sfd = socket(ptr->ai_family, \
			ptr->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC, ptr->ai_protocol);
		if (sfd == -1)
			continue;
		int re = 1L;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof re) == -1) {
			log << Log::ERROR << "Failed to set socket as reusable"
				<< std::endl;
			(void)close(sfd);
			return false;
		}
		if (bind(sfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
			(void)close(sfd);
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (!ptr) {
		log << Log::ERROR << "Failed to initialize server socket" << std::endl;
		return false;
	}
	if (listen(sfd, SOMAXCONN) == -1) {
		log << Log::ERROR << "Failed to listen on bound socket" << std::endl;
		(void)close(sfd);
		return false;
	}
	listen_fd_ = sfd;
	return true;
}

void Server::add_client(int listen_fd)
{
	// We default to be anonymous accepting (non-tracking) server
	int client_fd = STRICT_EVALUATOR ? \
				accept(listen_fd, NULL, NULL) : \
				accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (client_fd == -1) {
		log << Log::ERROR << "Failed to accept connection" << std::endl;
		return ;
	}
	if (STRICT_EVALUATOR)
		fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, client_fd, &event)) {
		log << Log::ERROR << "Failed to add client_fd to epoll_ctl"
			<< std::endl;
		(void)close(client_fd);
	} else if (!this->clients_.insert(client_fd).second) {
		log << Log::ERROR << "Failed storing client fd: " << client_fd
			<< std::endl;
		this->close_connection(client_fd);
	} else {
		log << Log::INFO << "Client connection: " << client_fd << std::endl;
	}
}

void Server::close_connection(int fd)
{
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
		log << Log::ERROR << "Failed to remove fd from epoll" << std::endl;
	}
	(void)close(fd);
	std::map<int, Request*>::iterator it = this->requests_.find(fd);
	if (it != this->requests_.end())
	{
		delete it->second;
		this->requests_.erase(it);
	}
	this->replies_.erase(fd);
	this->clients_.erase(fd);
	log << Log::INFO << "Closed connection with client: " << fd << std::endl;
}

void Server::recv_request(int fd)
{
	static int const	len = 4096;
	char				buff[len];
	std::string			msg;

	ssize_t r = recv(fd, buff, len, MSG_DONTWAIT);
	switch (r) {
		case -1:
			log << Log::WARN << "recv client " << fd << " returned error"
				<< std::endl;
			return ;
		case 0:
			log << Log::INFO << "Client closed connection" << std::endl;
			this->close_connection(fd);
			return ;
		default:
			msg = std::string(buff, r);
	}
	if (DEBUG_MODE)
		log << Log::DEBUG << "Received " << r << "b: " << msg << std::endl;

	std::map<int, Request*>::iterator it = this->requests_.find(fd);
	if (it == this->requests_.end())
	{
		this->requests_[fd] = new Request(msg, this->log);
		int status = this->requests_[fd]->validate_request_line();
		if (!status)
			return ;
		this->replies_[fd] = Reply::get_status_line(status);
		log << Log::DEBUG << this->replies_[fd] << std::endl;
		it = this->requests_.find(fd);
		if (it != this->requests_.end())
		{
			delete it->second;
			this->requests_.erase(it);
		}
	}
	else
	{
		it->second->append(msg);
	}

	epoll_event event = {};
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, fd, &event)) {
		log << Log::ERROR << "Failed to modify polling" << std::endl;
		this->close_connection(fd);
	}
}

void Server::send_reply(int fd)
{
	ssize_t		s;
	std::string	rep;

	std::map<int, std::string>::iterator it = this->replies_.find(fd);
	if (it == this->replies_.end())
		return ;
	rep = it->second;
	s = send(fd, rep.c_str(), rep.length(), MSG_DONTWAIT);
	log << Log::DEBUG << "Sent " << s << ": " << rep.c_str();
	if (s < 0) {
		log << Log::ERROR << "Failed to send message" << std::endl;
		this->close_connection(fd);
		return ;
	}
	if (static_cast<size_t>(s) < rep.length()) {
		it->second.erase(0, s);
		return ;
	}
	this->replies_.erase(fd);
	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, fd, &event)) {
		log << Log::ERROR << "Failed to modify polling" << std::endl;
		this->close_connection(fd);
	}
}

void Server::cleanup()
{
	return ;
}
