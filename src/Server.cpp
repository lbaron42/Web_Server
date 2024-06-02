/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:34:37 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/02 12:31:43 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <ostream>

////////////////////////////////////////////////////////////////////////////////
//	Functors
////////////////////////////////////////////////////////////////////////////////

struct DelValue
{
	void operator()(std::pair<int, Request*> pair)	{ delete pair.second; }
};

struct CompareLocLen
{
	bool operator()(
		ServerData::Location const &one,
		ServerData::Location const &two
	) const {
		return (one.location_path.length() > two.location_path.length());
	}
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
		client_max_body_size(1024 * 1024),
		autoindex(false),
		allow_methods(Request::NONE),
		cgi_path(),
		cgi_ext(),
		locations()
{}

ServerData::Address::Address()
	:	ip(),
		port()
{}

ServerData::Address::~Address()
{}

ServerData::Location::Location()
	:	location_path(),
		alias(),
		loc_index(),
		allow_methods(Request::NONE),
		autoindex(false),
		redirection()
{}

ServerData::Location::~Location()
{}

ServerData::~ServerData()
{}

Server::Server(ServerData const &server_data, Log &log)
	:	info(server_data),
		log(log),
		epoll_fd(-1),
		clients(),
		requests(),
		replies(),
		keep_alive(),
		cgis()
{}

Server::Server(Server const &rhs)
	:	info(rhs.info),
		log(rhs.log),
		epoll_fd(rhs.epoll_fd),
		clients(rhs.clients),
		requests(rhs.requests),
		replies(rhs.replies),
		keep_alive(rhs.keep_alive),
		cgis(rhs.cgis)
{}

Server::~Server()
{
	std::for_each(this->requests.begin(), this->requests.end(), DelValue());
}

////////////////////////////////////////////////////////////////////////////////
//	Getters/Setters
////////////////////////////////////////////////////////////////////////////////

const std::vector<ServerData::Address> Server::get_addresses() const
{
	return this->info.addresses;
}

const std::vector<std::string> Server::get_hostnames() const
{
	return this->info.hostnames;
}

std::vector<std::pair<int, CGIHandler*> > Server::get_cgi_pipes()
{
	std::vector<std::pair<int, CGIHandler*> >	result;

	while (!this->upstream_q.empty()) {
		std::pair<int, CGIHandler*> in = this->upstream_q.front();
		this->upstream_q.pop();
		if (this->upstream_q.empty())
			return result;
		std::pair<int, CGIHandler*> out = this->upstream_q.front();
		this->upstream_q.pop();
		result.push_back(in);
		result.push_back(out);
	}
	return result;
}

void Server::set_epoll(int epoll_fd)
{
	this->epoll_fd = epoll_fd;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Server::sort_locations(void)
{
	CompareLocLen comp;
	std::sort(this->info.locations.begin(), this->info.locations.end(), comp);
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
		}
		if (bind(sfd, ptr->ai_addr, ptr->ai_addrlen) == -1)
			(void)close(sfd);
		break;
	}
	freeaddrinfo(servinfo);
	if (!ptr) {
		log << Log::ERROR << "Failed to initialize server socket" << std::endl;
		return -1;
	}
	if (listen(sfd, SOMAXCONN) == -1) {
		log << Log::ERROR << "Failed to listen on bound socket" << std::endl;
		(void)close(sfd);
		return -1;
	}
	return sfd;
}

int Server::add_client(int listen_fd)
{
	// We default to be anonymous accepting (non-tracking) server
	int client_fd = STRICT_EVALUATOR ?
			accept(listen_fd, NULL, NULL) :
			accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (client_fd == -1) {
		log << Log::ERROR << "Failed to accept connection" << std::endl;
		return -1;
	}

	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &event)) {
		log << Log::ERROR << "Failed to add client_fd " << client_fd
			<< " to epoll_ctl" << std::endl;
		(void)close(client_fd);
		return -1;
	}
	if (!this->clients.insert(client_fd).second) {
		log << Log::WARN << "Already tracking connection with client fd: "
			<< client_fd << std::endl;
	}
	log << Log::INFO << "Client connected: " << client_fd << std::endl;
	return client_fd;
}

void Server::close_connection(int fd)
{
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
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

bool Server::recv_request(int fd,
		std::queue<std::pair<Request*, int> > &que)
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
			this->close_connection(fd);
			return true;
		default:
			msg = std::string(buff, r);
	}
	log << Log::DEBUG << "Received " << r << "b: "
		<< std::endl << msg << std::endl;

	std::map<int, CGIHandler*>::iterator cg = this->cgis.find(fd);
	if (cg != this->cgis.end()) {
		return cg->second->receive();
	}
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end()) {
		Request *request = new (std::nothrow) Request(msg, this->log);
		if (!request) {
			log << Log::ERROR << "Memory allocation failed!" << std::endl;
			this->internal_error(fd, 500);
			return !this->switch_epoll_mode(fd, EPOLLOUT);
		}
		this->requests.insert(std::make_pair(fd, request));
	} else {
		log << Log::DEBUG << "Appending to previous received request"
			<< std::endl;
		it->second->append(msg);
	}
	if (!this->requests[fd]->is_parsed())
		this->parse_request(fd);
	if (this->requests[fd]->is_parsed()) {
		if (!this->requests[fd]->is_bounced()
		&& !this->matches_hostname(this->requests[fd])) {
			log << Log::DEBUG << "Hostname doesn't match, bouncing request"
				<< std::endl;
			this->requests[fd]->set_bounced(true);
			que.push(std::make_pair(this->requests[fd], fd));
			this->requests.erase(fd);
			return false;
		}
		if (this->handle_request(fd))
			return !this->switch_epoll_mode(fd, EPOLLOUT);
	}
	return false;
}

bool Server::matches_hostname(Request *request)
{
	std::string const	hostname = request->get_header("host");
	if (hostname.empty())
		return !request->is_version_11();
	std::string::size_type	div = hostname.find(":");
	std::string	ip;
	std::string	port;
	if (div != std::string::npos) {
		ip = hostname.substr(0, div);
		port = hostname.substr(div + 1);
	} else {
		ip = hostname;
		port = std::string();
	}
	std::vector<ServerData::Address>::const_iterator ads;
	for (ads = this->info.addresses.begin();
	ads != this->info.addresses.end(); ++ads) {
		if (ads->ip == ip && (port.empty() || ads->port == port))
			return true;
	}
	std::vector<std::string>::const_iterator it = this->info.hostnames.begin();
	for ( ; it != this->info.hostnames.end(); ++it) {
		if (*it == ip)
			return true;
	}
	return false;
}

void Server::register_request(int client_fd, Request *request)
{
	if (!this->requests.count(client_fd))
		this->requests.insert(std::make_pair(client_fd, request));
	else
		log << Log::DEBUG << "Already assigned this request to this connection"
			<< std::endl;
}

bool Server::handle_request(int fd)
{
	Request				*request = this->requests[fd];
	Headers				hdrs;
	std::vector<char>	payload;

	if (request->is_bounced()) {
		std::string const host = request->get_header("host");
		if (host.empty() && request->is_version_11()) {
			request->set_status(400);
			this->prepare_error_page(request, hdrs, payload);
		}
	}
	if (this->is_cgi_request(request, hdrs)) {
		log << Log::DEBUG << "CGI request recognized" << std::endl;
		if (!this->handle_cgi(fd, request)) {
			if (this->requests.count(fd))
				this->prepare_error_page(request, hdrs, payload);
			else
				return true;
		} else
			return false;
	} else if (request->get_status() < 400) {
		switch (request->get_method()) {
			case Request::GET:
				this->get_payload(request, hdrs, &payload);
				break;
			case Request::HEAD:
				this->get_head(request, hdrs);
				break;
			case Request::POST:
				this->handle_post_request(request, hdrs, &payload);
				break;
			case Request::PUT:
				this->handle_put_request(request, hdrs, &payload);
				break;
			case Request::DELETE: 
				this->handle_delete_request(request);
				break;
			default:
				request->set_status(501); break;
		}
		if (!request->is_version_11()
		|| icompare(request->get_header("connection"), "close"))
			hdrs.set_header("Connection", "close");
		else
			hdrs.set_header("Connection", "keep-alive");

		if (request->get_status() < 400
		&& (request->get_method() & (Request::POST | Request::PUT))
		&& !request->is_body_loaded()) {
			log << Log::DEBUG << "Incomplete request body, returning to epoll"
				<< std::endl;
			return false;
		}
		if (request->get_status() >= 400)
			this->prepare_error_page(request, hdrs, payload);
	}

	log << Log::INFO << "Request from client: " << fd
		<< "	=> " << request->get_status() << " "
		<< Reply::get_status_message(request->get_status()) << std::endl
		<< "\t\t\t\t" << request->get_req_line() << std::endl;

	if (hdrs.get_header("Connection") != "close")
		this->keep_alive.insert(fd);
	else
		this->keep_alive.erase(fd);

	std::vector<char>	repl;
	Reply::generate_response(request, hdrs, payload, repl);
	this->enqueue_reply(fd, repl)
		.drop_request(fd)
		.check_queue(fd);
	return true;
}

// TODO: rework pipelining to properly sequence multiple request-replies
Server &Server::check_queue(int fd)
{
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end())
		return *this;
	this->handle_request(fd);
	return *this;
}

bool Server::switch_epoll_mode(int fd, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event)) {
		log << Log::ERROR << "Failed to modify polling for fd: "
			<< fd << std::endl;
		this->close_connection(fd);
		return false;
	}
	return true;
}

bool Server::send_reply(int fd)
{
	ssize_t	s;

	std::map<int, CGIHandler*>::iterator cg = this->cgis.find(fd);
	if (cg != this->cgis.end()) {
		return cg->second->send_reply();
	}
	std::map<int, std::vector<char> >::iterator it = this->replies.find(fd);
	if (it == this->replies.end()) {
		log << Log::WARN << "Un numero sbagliato" << std::endl;
		return false;
	}
	s = send(fd, it->second.data(), it->second.size(), MSG_DONTWAIT);
	log << Log::DEBUG << "Reply sent " << s << " bytes" << std::endl;
	if (s < 0) {
		log << Log::ERROR << "Failed to send message" << std::endl;
		this->close_connection(fd);
		return false;
	}
	if (static_cast<size_t>(s) < it->second.size()) {
		it->second.erase(it->second.begin(), it->second.begin() + s);
		return true;
	}
	this->replies.erase(fd);
	if (!this->keep_alive.count(fd)
	|| !this->switch_epoll_mode(fd, EPOLLIN))
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

bool Server::add_epoll_mode(int fd, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event)) {
		log << Log::ERROR << "Failed to adding fd to polling"
			<< std::endl;
		return false;
	}
	return true;
}

void Server::parse_request(int fd)
{
	Request	*request = this->requests[fd];
	request->set_status(request->validate_request_line());
	if (!request->get_status())
		return ;
	if (request->get_status() != 200) {
		request->set_parsed(true);
		return ;
	}
	if (!request->parse_headers())
		return ;
	request->set_parsed(true);
	// log << Log::DEBUG << "Parsed headers: " << std::endl
	// 	<< request->get_headers()
	// 	<< std::endl;
	return ;
}

// TODO: make check if data left in stream to be proc'd as new request
Server &Server::drop_request(int fd)
{
	std::map<int, Request*>::iterator it = this->requests.find(fd);
	if (it == this->requests.end())
		return *this;
	if (it->second->get_status() < 400 && !it->second->is_done()) {
		// Request *next_request = new (std::nothrow) Request(*it->second);
		// delete it->second;
		// if (!next_request) {
		// 	log << Log::ERROR << "Memory allocation failed!" << std::endl;
		// 	this->requests.erase(it);
		// 	// TODO: Send 500 Error ??
		// 	return *this;
		// }
		// this->requests[fd] = next_request;
		// log << Log::DEBUG << "More requests in queue. Dropping it anyway"
		// 	<< std::endl;
		delete it->second;
		this->requests.erase(it);
	} else {
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

std::string Server::resolve_address(Request *request, Headers &headers)
{
	std::string	path(this->info.root);
	std::string	url = request->get_url();

	std::vector<ServerData::Location> const	locs = this->info.locations;
	std::vector<ServerData::Location>::const_iterator	lo;
	for (lo = locs.begin(); lo != locs.end(); ++lo) {
		if (url.rfind(lo->location_path, 0) != std::string::npos) {
			log << Log::DEBUG << "Matched location: " << lo->location_path
				<< std::endl;
			if (lo->allow_methods
			&& !(request->get_method() & lo->allow_methods)) {
				log << Log::DEBUG << "Requested method:	"
					<< request->get_method() << std::endl
					<< "Allowed methods:	" << lo->allow_methods
					<< std::endl;
				request->set_status(405);
				return std::string();
			}
			if (!lo->redirection.empty()) {
				headers.set_header("Location", lo->redirection);
				request->set_status(308);
				return url;
			}
			if (!lo->alias.empty())
				url.replace(0, lo->location_path.length(), lo->alias);
			if (lo->autoindex) {
				request->set_dirlist(true);
				return (path + url);
			}
			if (try_file(path + url))
				return (path + url);
			if (!lo->loc_index.empty()) {
				std::vector<std::string>::const_iterator idx;
				if (url[url.length() - 1] != '/')
					url.append("/");
				for (idx = lo->loc_index.begin();
				idx != lo->loc_index.end(); ++idx) {
					if (try_file(path + url + "/" + *idx))
						return (path + url + "/" + *idx);
					if (try_file(path + url + *idx))
						return (path + url + *idx);
				}
			}
			return (path + url);
		}
	}
	if (!(request->get_method() & this->info.allow_methods)) {
		log << Log::DEBUG << "Requested method:	"
			<< request->get_method() << std::endl
			<< "Allowed methods:	" << this->info.allow_methods
			<< std::endl;
		request->set_status(405);
		return std::string();
	}
	url = request->get_url();
	if (this->info.autoindex) {
		request->set_dirlist(true);
		return (path + url);
	}
	if (try_file(path + url))
		return (path + url);
	if (!this->info.serv_index.empty()) {
		std::vector<std::string>::const_iterator idx;
		if (url[url.length() - 1] != '/')
			url.append("/");
		for (idx = this->info.serv_index.begin();
		idx != this->info.serv_index.end(); ++idx) {
			if (try_file(path + url + "/" + *idx))
				return (path + url + "/" + *idx);
			if (try_file(path + url + *idx))
				return (path + url + *idx);
		}
	}
	return (path + url);
}

void Server::get_head(Request *request, Headers &headers)
{
	std::string const	path = request->get_target();
	struct stat	statbuf;

	log << Log::DEBUG << "Resolved URL: [" << path << "]" << std::endl;
	if (request->get_status() >= 400)
		return ;
	if (path.empty()) {
		request->set_status(404);
		log << Log::DEBUG << "File not found" << std::endl;
		return ;
	}
	if (request->get_status() >= 400) {
		return ;
	}
	if (!stat(path.c_str(), &statbuf) && S_ISDIR(statbuf.st_mode)) {
		if (request->is_dirlist()) {
			headers.set_header("Content-Type", "text/html");
			headers.set_header("Content-Length",
					num_tostr(Reply::get_html_size(path, request->get_url())));
			request->set_status(200);
		} else {
			log << Log::DEBUG << "Requested file is a directory" << std::endl;
			request->set_status(404);
		}
		return ;
	}
	if (access(path.c_str(), F_OK)) {
		request->set_status(404);
		return ;
	}
	if (access(path.c_str(), R_OK)) {
		log << Log::DEBUG << "Cannot open requested file: "
			<< request->get_url() << std::endl;
		request->set_status(403);
		return ;
	}
	headers.set_header("Content-Type", get_mime_type(path));
	headers.set_header("Content-Length",
			num_tostr(get_file_size(path)));
	request->set_status(200);
}

void Server::get_payload(Request *request, Headers &headers,
		std::vector<char> *body)
{
	this->get_head(request, headers);
	if (request->get_status() < 400) {
		struct stat statbuf;
		if (!stat(request->get_target().c_str(), &statbuf)
		&& S_ISDIR(statbuf.st_mode)) {
			if (request->is_dirlist()) {
				std::string tmp = Reply::get_listing(request->get_target(),
						request->get_url());
				if (!tmp.empty())
					body->insert(body->end(), tmp.begin(), tmp.end());
				else {
					request->set_status(404);
					log << Log::DEBUG << "Failed to open dir" << std::endl;
				}
			} else {
				request->set_status(404);
				log << Log::DEBUG << "Requested file is a directory"
					<< request->get_target() << std::endl;
			}
		} else {
			*body = Reply::get_payload(request->get_target());
		}
	}
}

static inline bool get_body_size(std::string const &content_len,
		size_t *out_size)
{
	if (!is_uint(content_len))
		return false;
	*out_size = str_tonum<size_t>(content_len);
	return true;
}

bool Server::load_request_body(Request *request)
{
	size_t		body_size(0);

	if (!get_body_size(request->get_header("content-length"), &body_size)) {
		log << Log::DEBUG << "Content-Length value is not a valid number"
			<< std::endl;
		request->set_status(400);
		return false;
	}
	if (!request->get_loaded_body_size()
	&& body_size > this->info.client_max_body_size) {
		log << Log::DEBUG << "Content-Length exceeds max body size"
			<< std::endl;
		request->set_status(413);
		return false;
	}
	return request->load_payload(body_size - request->get_loaded_body_size());
}

void Server::handle_post_request(Request *request, Headers &headers,
		std::vector<char> *body)
{
	std::string				content_type(request->get_header("content-type"));
	std::string::size_type	div(content_type.find(";"));
	std::string::size_type	pos;
	std::string				type(content_type.substr(0, div));
	size_t					body_size(0);

	if (request->get_path().empty()) {
		request->set_path(this->resolve_address(request, headers));
		if (request->get_path().empty())
			request->set_status(400);
	}
	if (request->get_status() >= 400)
		return ;
	if (type == "application/x-www-form-urlencoded") {
		log << Log::DEBUG << "URL encoded form body" << std::endl;
		if (!request->is_body_loaded())
			this->load_request_body(request);
		if (!request->is_body_loaded())
			return ;
	} else if (type == "application/json") {
		log << Log::DEBUG << "JSON encoded form body" << std::endl;
		if (!request->is_body_loaded())
			this->load_request_body(request);
		if (!request->is_body_loaded())
			return ;
	} else if (type == "multipart/form-data" && div != std::string::npos
	&& ((pos = content_type.find("boundary=", div + 1)) != std::string::npos)) {
		std::string	boundary(trim(content_type.substr(pos + 9)));
		log << Log::DEBUG << "Multipart encoded form body" << std::endl
			<< "Boundary:	[" << boundary << "]" << std::endl;
		if (get_body_size(request->get_header("content-length"), &body_size)) {
			log << Log::DEBUG << "Content-Length value is not a valid number"
				<< std::endl;
			request->set_status(400);
			return ;
		}
		if (!request->get_loaded_body_size()
		&& body_size > this->info.client_max_body_size) {
			log << Log::DEBUG << "Content-Length exceeds max body size"
				<< std::endl;
			request->set_status(413);
			return ;
		}
		if (!request->load_multipart("--" + boundary, body_size)
		|| request->get_status() >= 400) {
			return ;
		}
	} else {
		request->set_status(400);
		return ;
	}
	// request->set_status(201);
	request->set_status(303);
	(void)body;
}

void Server::handle_put_request(Request *request, Headers &headers,
		std::vector<char> *body)
{
	if (!request->is_body_loaded())
		this->load_request_body(request);
	if (!request->is_body_loaded())
		return ;
	log << Log::DEBUG << "Loaded body size: " << request->get_loaded_body_size()
		<< std::endl;
	std::string	type(request->get_header("content-type"));
	bool		is_text(!type.rfind("text", 0));
	if (is_text)
		log << Log::DEBUG << "PUT Text file" << std::endl;
	else
		log << Log::DEBUG << "PUT Binary file" << std::endl;
	std::string	relative_location(request->get_url());
	if (relative_location.empty()) {
		request->set_status(400);
		return ;
	}
	std::string const	target(request->get_target());
	bool				file_exists(access(target.c_str(), F_OK) == 0);
	if (!file_exists || !access(target.c_str(), W_OK)) {
		std::ofstream	file;
		if (is_text)
			file.open(target.c_str(), std::ios::trunc);
		else
			file.open(target.c_str(), std::ios::trunc | std::ios::binary);
		if (!file.is_open()) {
			log << Log::DEBUG << "Failed to open file: " << target << std::endl;
			request->set_status(409);
			return ;
		}
		file.write(request->get_payload().data(),
				request->get_loaded_body_size());
		if (is_text)
			file << "\n";
		file.close();
		request->set_status(file_exists ? 200 : 201);
		if (!request->get_loaded_body_size())
			request->set_status(204); // success - no content
		headers.set_header("Content-Type", type);
		headers.set_header("Content-Location", request->get_url());
		if (file_exists) {
			std::string const	msg("Updated file " + request->get_url());
			body->insert(body->end(), msg.begin(), msg.end());
		} else {
			std::string const	msg("Created file " + request->get_url());
			body->insert(body->end(), msg.begin(), msg.end());
		}
	} else {
		log << Log::DEBUG << "Couldn't create/access file specified"
			<< std::endl;
		request->set_status(403);
	}
}

void Server::handle_delete_request(Request *request)
{
	std::string	path(request->get_target());
	struct stat	sb;

	if (request->get_status() >= 400)	return;
	if (stat(path.c_str(), &sb) || !S_ISREG(sb.st_mode)) {
		request->set_status(404);
		return ;
	}
	if (access(path.c_str(), F_OK)) {
		request->set_status(404);
		return ;
	}
	if (std::remove(path.c_str())) {
		request->set_status(403);
		return ;
	}
	log << Log::DEBUG << "File to remove: [" << path << "]" << std::endl;
	request->set_status(204);
}

bool Server::is_cgi_request(Request *request, Headers &headers)
{
	request->set_target(this->resolve_address(request, headers));

	if (request->get_status() >= 400
	|| request->get_url().rfind(this->info.cgi_path, 0) == std::string::npos
	|| this->info.cgi_path.empty() || this->info.cgi_ext.empty()) {
		log << Log::DEBUG << "CGI path: " << this->info.cgi_path << std::endl
			<< "URL: " << request->get_url() << std::endl;
		return false;
	}
	std::vector<std::string>::const_iterator it(this->info.cgi_ext.begin());
	for ( ; it != this->info.cgi_ext.end(); ++it) {
		if (ends_with(request->get_target(), *it))
			return true;
	}
	return false;
}

// TODO
bool Server::handle_cgi(int fd, Request *request)
{
	if (request->get_status() >= 400)
		return false;
	log << Log::DEBUG << "Running CGI: " << request->get_target() << std::endl;

	int			pipes[2];
	CGIHandler	*cgi = new (std::nothrow)
				CGIHandler(this->log, fd, this->info.client_max_body_size);
	if (!cgi) {
		log << Log::ERROR << "Memory allocation failed!" << std::endl;
		request->set_status(500);
		return false;
	}
	if (!cgi->execute(pipes, request)) {
		delete cgi;
		request->set_status(502);
		return false;
	}
	this->requests.erase(fd);
	if (this->add_epoll_mode(pipes[0], EPOLLIN)
	||	this->add_epoll_mode(pipes[1], EPOLLOUT)) {
		delete cgi;
		this->internal_error(fd, 500);
		return false;
	}
	this->cgis[fd] = cgi;
	this->upstream_q.push(std::make_pair(pipes[0], cgi));
	this->upstream_q.push(std::make_pair(pipes[1], cgi));
	return true;
}

void Server::prepare_error_page(Request *request, Headers &hdrs,
		std::vector<char> &payload)
{
	std::map<int, std::string>::const_iterator it = this
			->info.error_pages.find(request->get_status());
	if (it != this->info.error_pages.end()) {
		std::string path(this->info.root + "/" + it->second);
		payload = Reply::get_payload(path);
		hdrs.set_header("Content-Length", num_tostr(payload.size()));
	}
	if (payload.empty()) {
		std::string tmp = Reply::generate_error_page(request->get_status());
		payload.insert(payload.end(), tmp.begin(), tmp.end());
		hdrs.set_header("Content-Length", num_tostr(tmp.length()));
	}
	if (request->get_method() == Request::HEAD) {
		payload.clear();
	}
	hdrs.set_header("Content-Type", "text/html");
	hdrs.set_header("Connection", "close");
}

void Server::internal_error(int fd, int code)
{
	std::vector<char>	reply;
	std::string const	body = Reply::generate_error_page(code);

	std::stringstream	ss;
	ss << "HTTP/1.1 500 Internal Server Error\r\n"
		<< "Connection: close\r\n"
		<< "Content-Type: text/html\r\n"
		<< "\r\n";
	char c;
	while (ss >> c)
		reply.push_back(c);
	std::copy(body.begin(), body.end(), std::back_inserter(reply));
	this->keep_alive.erase(fd);
	(void)this->enqueue_reply(fd, reply);
}
