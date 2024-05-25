/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/25 04:52:42 by lbaron           ###   ########.fr       */
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

ServerData::ServerData()
	:	addresses(),
		hostnames(),
		error_pages(),
		serv_index(),
		root(),
		client_max_body_size(1024),
		autoindex(false),
		allow_methods(),
		locations()
{}

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
	for (AddrIter it = this->info.addresses.begin();
	it != this->info.addresses.end(); ++it) {
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
		log << Log::INFO << "Initialized server: " << this->info.hostnames[0]
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
	char				buff[4096];
	std::string			msg;

	ssize_t r = recv(fd, buff, sizeof(buff), MSG_DONTWAIT);
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
	this->handle_request(fd, this->parse_request(fd));
	if (!this->switch_epoll_mode(epoll_fd, fd, EPOLLOUT))
		return -1;
	return 0;
}

void Server::handle_request(int fd, int status)
{
	Request				*request = this->requests[fd];
	Headers				hdrs;
	std::vector<char>	payload;
	std::vector<char>	repl;

	if (status == 200) {
		switch (request->get_method()) {
			case Request::GET:
				status = this->handle_get_request(request, hdrs, &payload);
				break;
			// case Request::POST: break;
			// case Request::DELETE: break;
			case Request::HEAD:
				status = 200; break;
			default:
				status = 400; break;
		}
		hdrs.set_header("Connection", "keep-alive");
	}
	if (status != 200) {
		hdrs.set_header("Content-Type", "text/plain");
		hdrs.set_header("Connection", "close");
	}

	log << Log::INFO << "Request from client: " << fd << std::endl
		<< "\t\t\t\t" << request->get_req_line() << " => "
		<< status << " " << Reply::get_status_message(status)
		<< std::endl;

	std::string tmp(Reply::get_status_line(request->is_version_11(), status));
	repl.insert(repl.end(), tmp.begin(), tmp.end());

	std::stringstream ss;
	ss << payload.size();
	hdrs.set_header("Content-Length", ss.str());
	ss.clear();
	ss.str(std::string());
	ss << hdrs;
	ss >> std::noskipws;
	char c;
	while (ss >> c)
		repl.push_back(c);
	repl.push_back('\r');
	repl.push_back('\n');
	if (!payload.empty()) {
		repl.insert(repl.end(), payload.begin(), payload.end());
		repl.push_back('\r');
		repl.push_back('\n');
	}
	repl.push_back('\r');
	repl.push_back('\n');

	this->drop_request(fd)
		.enqueue_reply(fd, repl);
}

int Server::send_reply(int epoll_fd, int fd)
{
	ssize_t	s;

	std::map<int, std::vector<char> >::iterator it = this->replies.find(fd);
	if (it == this->replies.end()) {
		log << Log::WARN << "Un numero sbagliato" << std::endl;
		return 0;
	}
	s = send(fd, it->second.data(), it->second.size(), MSG_DONTWAIT);
	log << Log::DEBUG << "Reply sent " << s << " bytes" << std::endl;
	if (s < 0) {
		log << Log::ERROR << "Failed to send message" << std::endl;
		this->close_connection(epoll_fd, fd);
		return -1;
	}
	if (static_cast<size_t>(s) < it->second.size()) {
		it->second.erase(it->second.begin(), it->second.begin() + s);
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
	// if (!(this->requests[fd]->get_method() & this->info.allow_methods))
	// 	status = 405;
	return status;
}

Server &Server::drop_request(int fd)
{
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it != this->requests.end())
	{
		delete it->second;
		this->requests.erase(it);
	}
	return *this;
}

Server &Server::enqueue_reply(int fd, std::vector<char> const &reply)
{
	std::map<int, std::vector<char> >::iterator rip = this->replies.find(fd);
	if (rip == this->replies.end())
		this->replies.insert(std::make_pair(fd, reply));
	else
		rip->second.insert(rip->second.end(), reply.begin(), reply.end());
	return *this;
}

int Server::handle_get_request(Request *request, Headers &headers,
		std::vector<char> *body)
{
	std::string const	&url = request->get_url();
	std::string			path(this->info.root);
	std::string			msg_body;

	if (url[url.size() - 1] != '/') {
		path.append(url);
	} else if (this->info.autoindex) {
		// TODO: Check if permissions allow access
		msg_body = Reply::get_listing(url);
		std::copy(msg_body.begin(), msg_body.end(), std::back_inserter(*body));
		headers.set_header("Content-Type", "text/html");
		return 200;
	} else if (url == "/" && !this->info.serv_index.empty()) {
		path.append(url);
		path.append(this->info.serv_index[0]);
	} else {
		return 400; // TODO: confirm status code for this case, maybe 404?
	}
	log << Log::DEBUG << "Requested file: " << path << std::endl;
	if (access(path.c_str(), F_OK))
		return 404;
	if (access(path.c_str(), R_OK)) {
		log << Log::ERROR << "Cannot open requested file: "
			<< url << std::endl;
		return 403;
	}
	headers.set_header("Content-Type", get_mime_type(path));
	if (!get_mime_type(path).rfind("text", 0)) {
		msg_body = Reply::get_content(path);
		std::copy(msg_body.begin(), msg_body.end(), std::back_inserter(*body));
	} else {
		*body = Reply::get_payload(path);
	}
	return 200;
}
