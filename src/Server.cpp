/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 01:53:53 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Functors
////////////////////////////////////////////////////////////////////////////////

struct DelValue
{
	void operator()(std::pair<int, Request*> pair)	{ delete pair.second; }
};

struct AssignFd
{
	AssignFd(Server const *s) : serv(s) {}

	std::pair<int, Server const*> operator()(int fd) const
	{
		return std::make_pair(fd, serv);
	}

	private:
		Server const	*serv;
};

////////////////////////////////////////////////////////////////////////////////
//	CTORs/DTOR
////////////////////////////////////////////////////////////////////////////////

Server::Server(ServerData const &server_data, Log &log)
	: info(server_data), log(log)
{}

Server::~Server()
{
	std::for_each(this->requests.begin(), this->requests.end(), DelValue());
}

Server::Server(Server const &rhs) : info(rhs.info), log(rhs.log)
{}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

bool Server::initialize(int epoll_fd)
{
	typedef std::vector<ServerData::Address>::const_iterator AddrIter;
	for (AddrIter it = this->info.address.begin();
	it != this->info.address.end(); ++it) {
		int sfd = setup_socket(it->port.c_str(), \
				it->ip.empty() ? NULL : it->ip.c_str());
		if (sfd == -1)
			return false;
		this->listen_fds.insert(sfd);
		log << Log::DEBUG << "Listening on: " << it->ip.c_str() << ":"
			<< it->port.c_str() << " | File desc: " << sfd
			<< std::endl;
	}

	epoll_event event = {};
	event.events = EPOLLIN;
	for (std::set<int>::const_iterator it = this->listen_fds.begin();
	it != this->listen_fds.end(); ++it) {
		event.data.fd = *it;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &event)) {
			log << Log::ERROR << "Failed to add fd " << *it 
				<< " to epoll_ctl" << std::endl;
			return false;
		}
		log << Log::DEBUG << "Added fd " << *it << " to epoll_ctl" << std::endl;
		log << Log::INFO << "Initialized server: " << this->info.hostname[0]
			<< std::endl;
	}
	return true;
}

std::map<int, Server const*> Server::get_listen_fds() const
{
	AssignFd						assign_fd(this);
	std::map<int, Server const*>	listen_map;

	std::transform(this->listen_fds.begin(), this->listen_fds.end(), \
			std::inserter(listen_map, listen_map.end()), assign_fd);
	return listen_map;
}

int Server::add_client(int epoll_fd, int listen_fd)
{
	// We default to be anonymous accepting (non-tracking) server
	int client_fd = STRICT_EVALUATOR ? \
			accept(listen_fd, NULL, NULL) : \
			accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (client_fd == -1) {
		log << Log::ERROR << "Failed to accept connection" << std::endl;
		return -1;
	}
	if (STRICT_EVALUATOR)	fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event)) {
		log << Log::ERROR << "Failed to add client_fd " << client_fd
			<< " to epoll_ctl" << std::endl;
		(void)close(client_fd);
		return -1;
	}
	if (!this->clients.insert(client_fd).second) {
		log << Log::ERROR << "Failed storing client fd: " << client_fd
			<< std::endl;
		this->close_connection(epoll_fd, client_fd);
		return -1;
	}
	log << Log::INFO << "Client connected: " << client_fd << std::endl;
	return client_fd;
}

void Server::close_connection(int epoll_fd, int fd)
{
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		log << Log::ERROR << "Failed to remove fd from epoll" << std::endl;
	}
	(void)close(fd);
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it != this->requests.end()) {
		delete it->second;
		this->requests.erase(it);
	}
	this->replies.erase(fd);
	this->clients.erase(fd);
	log << Log::INFO << "Closed connection with client: " << fd << std::endl;
}

int Server::recv_request(int epoll_fd, int fd)
{
	static int const	len = 4096;
	char				buff[len];
	std::string			msg;

	ssize_t r = recv(fd, buff, len, MSG_DONTWAIT);
	switch (r) {
		case -1:
			log << Log::WARN << "recv client " << fd << " returned error"
				<< std::endl;
			return 0;
		case 0:
			log << Log::INFO << "Client closed connection" << std::endl;
			this->close_connection(epoll_fd, fd);
			return -1;
		default:
			msg = std::string(buff, r);
	}
	if (DEBUG_MODE)
		log << Log::DEBUG << "Received " << r << "b: " << msg << std::endl;

	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end())
		this->requests[fd] = new Request(msg, this->log);
	else
		it->second->append(msg);

	int status = this->parse_request(fd);

	Headers		hdrs;
	std::string	msg_body;
	std::string filepath(this->info.root);
	if (status == 200) {
		switch (this->requests[fd]->get_method()) {
			case Request::HEAD:
				msg_body = std::string();
				break;
			case Request::GET:
				if (this->requests[fd]->get_url() != "/")
					filepath.append(this->requests[fd]->get_url());
				else {
					filepath.append("/");
					filepath.append(this->info.index);
				}
				if (access(filepath.c_str(), F_OK)) {
					status = 404;
				} else if (access(filepath.c_str(), R_OK)) {
					log << Log::ERROR << "Cannot open requested file: "
						<< this->requests[fd]->get_url() << std::endl;
					status = 403;
				} else {
					msg_body = Reply::get_content(filepath);
				}
				log << Log::DEBUG << "Requested file: " << filepath << std::endl;
				break;
			// case Request::POST: break;
			// case Request::DELETE: break;
			default:
				status = 400; break;
		}
	}
	hdrs.set_header("Connection", "Keep-Alive");
	hdrs.set_header("Content-Type", this->get_mime_type(filepath));
	std::ostringstream ss;
	ss << msg_body.size();
	hdrs.set_header("Content-Length", ss.str());
	ss.clear();
	ss.str(std::string());
	ss << Reply::get_status_line(this->requests[fd]->is_version_11(), status)
		<< hdrs << "\r\n\r\n" << msg_body << "\r\n\r\n";

	this->replies[fd] = ss.str();
	it = this->requests.find(fd);
	if (it != this->requests.end())
	{
		delete it->second;
		this->requests.erase(it);
	}
	if (!this->switch_epoll_mode(epoll_fd, fd, EPOLLOUT))
		return -1;
	return 0;
}

std::string const Server::get_mime_type(std::string const &file) const
{
	std::string::size_type	pos = file.find_last_of('.');

	if (pos == std::string::npos) {
		return std::string("text/plain");
	}
	std::string	ext = file.substr(pos + 1);
	if (ext == "html")
		return std::string("text/html");
	if (ext == "css")
		return std::string("text/css");
	if (ext == "js")
		return std::string("text/javascript");
	if (ext == "md")
		return std::string("text/markdown");
	if (ext == "html")
		return std::string("text/html");
	return std::string("text/plain");
}

int Server::parse_request(int fd)
{
	int status = this->requests[fd]->validate_request_line();
	if (status != 200)
		return status;
	if (!this->requests[fd]->parse_headers()) {
		log << Log::WARN << "Failed parsing headers" << std::endl;
		log << Log::DEBUG << "Recognized: " << std::endl
			<< this->requests[fd]->get_headers()
			<< std::endl;
		return 0;
	}
	log << Log::DEBUG << "Request headers: " << std::endl
		<< this->requests[fd]->get_headers()
		<< std::endl;
	if (!(this->requests[fd]->get_method() & this->info.allowed_methods))
		status = 405;
	return status;
}

int Server::send_reply(int epoll_fd, int fd)
{
	ssize_t		s;
	std::string	rep;

	std::map<int, std::string>::iterator it = this->replies.find(fd);
	if (it == this->replies.end())
		return 0;
	rep = it->second;
	s = send(fd, rep.c_str(), rep.length(), MSG_DONTWAIT);
	log << Log::DEBUG << "Sent " << s << ": " << rep.c_str();
	if (s < 0) {
		log << Log::ERROR << "Failed to send message" << std::endl;
		this->close_connection(epoll_fd, fd);
		return -1;
	}
	if (static_cast<size_t>(s) < rep.length()) {
		it->second.erase(0, s);
		return 0;
	}
	this->replies.erase(fd);
	if (!this->switch_epoll_mode(epoll_fd, fd, EPOLLIN))
		return -1;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

bool Server::switch_epoll_mode(int epoll_fd, int fd, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event)) {
		log << Log::ERROR << "Failed to modify polling for fd "
			<< fd << std::endl;
		this->close_connection(epoll_fd, fd);
		return false;
	}
	return true;
}

int Server::setup_socket(char const *service, char const *node)
{
	int			sfd(-1);
	addrinfo	*servinfo;
	addrinfo	*ptr;
	addrinfo	hints = {};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (int ret = getaddrinfo(node, service, &hints, &servinfo)) {
		log << Log::ERROR << "getaddrinfo error: " \
			<< gai_strerror(ret) << std::endl;
		return -1;
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
			return -1;
		}
		if (bind(sfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
			(void)close(sfd);
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (!ptr) {
		log << Log::ERROR << "Failed to initialize server socket on port: "
			<< service << std::endl;
		return -1;
	}
	if (listen(sfd, SOMAXCONN) == -1) {
		log << Log::ERROR << "Failed to listen on bound socket" << std::endl;
		(void)close(sfd);
		return -1;
	}
	return sfd;
}
