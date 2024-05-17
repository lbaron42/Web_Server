/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 02:57:46 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

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
	: name_(name), port_(port), log(logger)
{}

Server::~Server()
{}

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
			log << "[ERROR] " << "Failed to create epoll file descriptor" << std::endl;
			this->cleanup();
			return false;
		}
		fcntl(this->epoll_fd_, F_SETFL, O_CLOEXEC);
	}
	else
	{
		if ((this->epoll_fd_ = epoll_create1(EPOLL_CLOEXEC)) == -1) {
			log << "[ERROR] " << "Failed to create epoll file descriptor" << std::endl;
			this->cleanup();
			return false;
		}
	}

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = listen_fd_;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event)) {
		log << "[ERROR] " << "Failed to add fd to epoll_ctl" << std::endl;
		this->cleanup();
		return false;
	}
	std::cout << "Server initialized: "
			<< this->name_ << ":" << this->port_ << std::endl;
	return true;
}

void Server::start()
{
	int const	timeout = 420;
	int const	max_events = 32;
	epoll_event	events[max_events];

	std::cout << "Listening for connections..." << std::endl;
	while (!marvinX::g_stopme) {
		int n_events = epoll_wait(this->epoll_fd_, events, max_events, timeout);
		if (n_events == -1) {
			if (!marvinX::g_stopme)
				log << "[ERROR] " << "Failed epoll_wait" << std::endl;
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
	std::cout << "[SIG] Server shutting down" << std::endl;
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
		log << "[ERROR] " << "getaddrinfo error: " \
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
			log << "[ERROR] " << "Failed to set socket as reusable" << std::endl;
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
		log << "[ERROR] " << "Failed to initialize server socket" << std::endl;
		return false;
	}
	if (listen(sfd, SOMAXCONN) == -1) {
		log << "[ERROR] " << "Failed to listen on bound socket" << std::endl;
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
		log << "[ERROR] " << "Failed to accept connection" << std::endl;
		return ;
	}
	if (STRICT_EVALUATOR)
		fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, client_fd, &event)) {
		log << "[ERROR] " << "Failed to add client_fd to epoll_ctl" << std::endl;
		(void)close(client_fd);
	} else if (!this->clients_.insert(client_fd).second) {
		log << "[ERROR] " << "Failed storing client fd: " << client_fd << std::endl;
		this->close_connection(client_fd);
	} else {
		std::cout << "Client connection: " << client_fd << std::endl;
	}
}

void Server::close_connection(int fd)
{
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
		log << "[ERROR] " << "Failed to remove fd from epoll" << std::endl;
	}
	(void)close(fd);
	this->clients_.erase(fd);
	std::cout << "Closed connection with client: " << fd << std::endl;
}

void Server::recv_request(int fd)
{
	static int const	len = 4097;
	char				buff[len];

	ssize_t r = recv(fd, buff, len - 1, MSG_DONTWAIT);
	switch (r) {
		case -1:
			return ;
		case 0:
			std::cout << "Client closed connection" << std::endl;
			this->close_connection(fd);
			return ;
		default:
			buff[r] = '\0';
	}
	std::cout << "Received " << r << ": " << buff << std::endl;

	epoll_event event = {};
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, fd, &event)) {
		log << "[ERROR] " << "Failed to modify polling" << std::endl;
		this->close_connection(fd);
	}
}

void Server::send_reply(int fd)
{
	ssize_t		s;
	std::string	rep;

	rep = "HTTP/1.1 200 OK\r\n\r\n";
	s = send(fd, rep.c_str(), rep.length(), MSG_DONTWAIT);
	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, fd, &event)) {
		log << "[ERROR] " << "Failed to modify polling" << std::endl;
		this->close_connection(fd);
	}
}

void Server::cleanup()
{
	return ;
}
