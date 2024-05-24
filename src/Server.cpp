/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/24 02:32:21 by mcutura          ###   ########.fr       */
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
	:	info(server_data),
		log(log),
		listen_fds(),
		clients(),
		requests(),
		replies()
{}

Server::~Server()
{
	std::for_each(this->requests.begin(), this->requests.end(), DelValue());
}

Server::Server(Server const &rhs)
	:	info(rhs.info),
		log(rhs.log),
		listen_fds(rhs.listen_fds),
		clients(rhs.clients),
		requests(rhs.requests),
		replies(rhs.replies)
{}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

bool Server::initialize(int epoll_fd)
{
	typedef std::vector<ServerData::Address>::const_iterator AddrIter;
	for (AddrIter it = this->info.address.begin();
	it != this->info.address.end(); ++it) {
		int sfd = this->setup_socket(it->port.c_str(), \
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

bool Server::recv_request(int epoll_fd, int fd)
{
	char				buff[4096];
	std::string			msg;

	ssize_t r = recv(fd, buff, sizeof(buff), MSG_DONTWAIT);
	switch (r) {
		case -1:
			log << Log::WARN << "recv client " << fd << " returned error"
				<< std::endl;
			return false;
		case 0:
			log << Log::INFO << "Client closed connection" << std::endl;
			this->close_connection(epoll_fd, fd);
			return true;
		default:
			msg = std::string(buff, r);
	}
	log << Log::DEBUG << "Received " << r << "b: " << msg << std::endl;

	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end()) {
		Request *request = new (std::nothrow) Request(msg, this->log);
		if (!request) {
			log << Log::ERROR << "Memory allocation failed!" << std::endl;
			// TODO: Send 500 Error ??
			this->close_connection(epoll_fd, fd);
			return true;
		}
		this->requests.insert(std::make_pair(fd, request));
	} else {
		log << Log::DEBUG << "Appending to previous received request"
			<< std::endl;
		it->second->append(msg);
	}
	this->parse_request(fd);
	if (this->requests[fd]->is_parsed()) {
		this->handle_request(fd);
		if (!this->switch_epoll_mode(epoll_fd, fd, EPOLLOUT))
			return true;
	}
	return false;
}

void Server::handle_request(int fd)
{
	Request				*request = this->requests[fd];
	int					status = request->get_status();
	Headers				hdrs;
	std::vector<char>	payload;
	std::vector<char>	repl;

	if (request->get_status() < 400) {
		switch (request->get_method()) {
			case Request::GET:
				this->get_payload(request, hdrs, &payload);
				break;
			case Request::HEAD:
				this->get_head(request, hdrs);
				break;
			case Request::POST:
				this->handle_post_request(request, hdrs);
				break;
			// case Request::DELETE: break;
			default:
				status = 400; break;
		}
		if (!request->is_version_11()
		|| request->get_header("Connection") == "close"
		|| request->get_header("Connection") == "Close"
		|| request->get_header("connection") == "close"
		|| request->get_header("connection") == "Close")
			hdrs.set_header("Connection", "close");
		else
			hdrs.set_header("Connection", "keep-alive");
	}
	if (request->get_status() >= 400) {
		if (request->get_method() != Request::HEAD) {
			std::string tmp = Reply::generate_error_page(request->get_status());
			payload.insert(payload.end(), tmp.begin(), tmp.end());
		}
		hdrs.set_header("Content-Length",
				num_tostr(Reply::get_html_size(request->get_status())));
		hdrs.set_header("Content-Type", "text/html");
		hdrs.set_header("Connection", "close");
	}

	log << Log::INFO << "Request from client: " << fd
		<< "	=> " << status << " " << Reply::get_status_message(status)
		<< std::endl << "\t\t\t\t" << request->get_req_line()
		<< std::endl;

	this->generate_response(request, hdrs, payload, repl)
		.enqueue_reply(fd, repl)
		.drop_request(fd)
		.check_queue(fd);
}

int Server::send_reply(int epoll_fd, int fd)
{
	ssize_t	s;

	std::map<int, std::vector<char> >::iterator it = this->replies.find(fd);
	if (it == this->replies.end()) {
		log << Log::WARN << "Un numero sbagliato" << std::endl;
		return -1;
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

void Server::parse_request(int fd)
{
	Request	*request = this->requests[fd];
	if (!request->get_status())
		request->set_status(request->validate_request_line());
	if (!request->get_status())
		return ;
	if (request->get_status() != 200) {
		request->set_parsed(true);
		return ;
	}
	if (!request->parse_headers()) {
		log << Log::DEBUG << "Received headers: " << std::endl
			<< request->get_headers()
			<< std::endl;
		return ;
	}
	request->set_parsed(true);
	log << Log::DEBUG << "Parsed headers: " << std::endl
		<< request->get_headers()
		<< std::endl;
	log << Log::DEBUG << "Current status: " << request->get_status()
		<< std::endl;
	if (!(request->get_method() & this->info.allowed_methods)) {
		log << Log::DEBUG << "Requested method:	" << request->get_method()
			<< std::endl;
		log << Log::DEBUG << "Allowed methods:	" << this->info.allowed_methods
			<< std::endl;
		request->set_status(405);
	}
}

Server &Server::drop_request(int fd)
{
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end())
		return *this;
	if (it->second->get_status() < 400 && !it->second->is_done()) {
		Request *next_request = new (std::nothrow) Request(*it->second);
		delete it->second;
		if (!next_request) {
			log << Log::ERROR << "Memory allocation failed!" << std::endl;
			this->requests.erase(it);
			// TODO: Send 500 Error ??
			return *this;
		}
		this->requests[fd] = next_request;
	} else {
		delete it->second;
		this->requests.erase(it);
	}
	return *this;
}

Server &Server::check_queue(int fd)
{
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end())
		return *this;
	this->handle_request(fd);
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

std::string Server::resolve_address(Request *request)
{
	std::string const	&url = request->get_url();
	std::string			path(this->info.root);

	if (url[url.size() - 1] == '/') {
		if (this->info.directory_listing) {
			request->set_dirlist(true);
		} else if (!this->info.index.empty()) {
			path.append(url);
			path.append(this->info.index);
		} else {
			return std::string();
		}
	} else {
		if (url[0] != '/')
			path.append("/");
		path.append(url);
	}
	return path;
}

void Server::get_head(Request *request, Headers &headers)
{
	std::string	path = this->resolve_address(request);

	log << Log::DEBUG << "Resolved URL: [" << path << "]" << std::endl;
	if (path.empty() || access(path.c_str(), F_OK)) {
		request->set_status(404);
		return ;
	}
	if (access(path.c_str(), R_OK)) {
		log << Log::ERROR << "Cannot open requested file: "
			<< request->get_url() << std::endl;
		request->set_status(403);
		return ;
	}
	if (request->is_dirlist()) {
		headers.set_header("Content-Type", "text/html");
		headers.set_header("Content-Length",
				num_tostr(Reply::get_html_size(path)));
	} else {
		headers.set_header("Content-Type", get_mime_type(path));
		headers.set_header("Content-Length",
				num_tostr(get_file_size(path)));
	}
	request->set_target(path);
	request->set_status(200);
}

void Server::get_payload(Request *request, Headers &headers,
		std::vector<char> *body)
{
	this->get_head(request, headers);
	if (request->get_status() < 400) {
		if (request->is_dirlist()) {
			std::string tmp = Reply::get_listing(request->get_url());
			body->insert(body->end(), tmp.begin(), tmp.end());
		} else
			*body = Reply::get_payload(request->get_target());
	} else {
		std::string tmp = Reply::generate_error_page(request->get_status());
		body->insert(body->end(), tmp.begin(), tmp.end());
	}
}

int Server::handle_post_request(Request *request, Headers &headers)
{

	std::string type = request->get_header("Content-Type");
	std::string body_size = request->get_header("Content-Length");

	if (type == "application/x-www-form-urlencoded") {

	}
	headers.set_header("Connection", "close");
	return 201;
}

Server &Server::generate_response(Request *request, Headers &headers,
		std::vector<char> const &body, std::vector<char> &repl)
{
	std::string tmp = Reply::get_status_line(request->is_version_11(),
			request->get_status());
	repl.insert(repl.end(), tmp.begin(), tmp.end());

	std::stringstream ss;
	ss >> std::noskipws;
	ss << headers << "\r\n";
	char c;
	while (ss >> c)
		repl.push_back(c);
	if (!body.empty()) {
		repl.insert(repl.end(), body.begin(), body.end());
		repl.push_back('\r');
		repl.push_back('\n');
	}
	repl.push_back('\r');
	repl.push_back('\n');
	return *this;
}
