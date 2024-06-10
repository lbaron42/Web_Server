/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plandolf <plandolf@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 17:57:12 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/10 12:05:55 by plandolf         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"
#include "Server.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTORs/DTOR
////////////////////////////////////////////////////////////////////////////////

CGIHandler::CGIHandler(Log &log, int client, size_t max_body_size, Server *owner)
	:	log(log),
		request(NULL),
		client_fd(client),
		input(-1),
		output(-1),
		pid(-1),
		status(0),
		wbuf(),
		rbuf(),
		headers(),
		reply(),
		max_body_size(max_body_size),
		owner(owner),
		chunkster(NULL)
{}

CGIHandler::CGIHandler(const CGIHandler &src)
	:	log(src.log),
		request(src.request),
		client_fd(src.client_fd),
		input(src.input),
		output(src.output),
		pid(src.pid),
		status(src.status),
		wbuf(src.wbuf),
		rbuf(src.rbuf),
		headers(src.headers),
		reply(src.reply),
		max_body_size(src.max_body_size),
		owner(src.owner),
		chunkster(src.chunkster)
{}

CGIHandler::~CGIHandler()
{
	if (this->input > -1)
		close(this->input);
	if (this->output > -1)
		close(this->output);
	if (this->pid > 1)
		kill(this->pid, SIGKILL);
	if (this->request)
		delete this->request;
	if (this->chunkster)
		delete this->chunkster;
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

/* TODO:
 * Write fork and exec part
 * No read, write or other blocking calls allowed
 * If succesful, CGIHandler assumes ownership of the Request*
 * and should delete it when done with it
 * @return: true if exec successful
 */
std::string CGIHandler::path_info(Request *request) 
{
	std::string url = request->get_url();
	std::vector<std::string> extensions;
	extensions.push_back(".cgi");
	extensions.push_back(".py");
	size_t start = std::string::npos;
	size_t extensionPos = std::string::npos;
		
	for (size_t i = 0; i < extensions.size(); ++i) {
		extensionPos = url.find(extensions[i]);
		if (extensionPos != std::string::npos) {
			start = extensionPos + extensions[i].length();
			break;
		}
	}
	if (start == std::string::npos)
		return "";
	size_t end = url.find('?', start);
	if (end == std::string::npos)
		return "";
	if (start == end)
		return "";
	std::string path = url.substr(start, end - start);
	return path;
}

bool CGIHandler::execute(int pipes[2], Request *request)
{
	std::vector<char const *>	argv;
	std::string					cmd(request->get_target());
	if (cmd.empty())
		return false;
	argv.push_back(cmd.c_str());
	argv.push_back(static_cast<char const *>(0));

	Headers	hdrs(request->get_headers());
	std::vector<char const *> env;
	// TODO: extract path after script name
	// i.e. if URL=/cgi-bin/script.cgi/additional/path?id=value&foo=bar
	// PATH_INFO=/additional/path
	// QUERY_STRING=id=value&foo=bar
	std::string path("PATH_INFO=");
	path.append(path_info(request));
	std::cout << path << std::endl;
	env.push_back(path.c_str());
	std::string method("REQUEST_METHOD=");
	method.append(request->get_method_as_str());
	env.push_back(method.c_str());
	std::string query("QUERY_STRING=");
	query.append(request->get_query());
	env.push_back(query.c_str());
	std::string gateway("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back(gateway.c_str());
	std::string protocol("SERVER_PROTOCOL=HTTP");
	env.push_back(protocol.c_str());
	env.push_back(static_cast<char const *>(0));

	int pin[2], pout[2];
	if (pipe(pin) || pipe(pout))
		return false;
	log << Log::DEBUG << "CGI Created pipe fds: "
		<< pin[0] << " " << pin[1] << " "
		<< pout[0] << " " << pout[1] << std::endl;
	this->pid = fork();
	if (this->pid < 0)
		return false;
	if (!this->pid) {
		dup2(pout[0], STDIN_FILENO);
		dup2(pin[1], STDOUT_FILENO);
		close(pin[0]);
		close(pin[1]);
		close(pout[0]);
		close(pout[1]);
		execve(argv[0],
			const_cast<char* const*>(argv.data()),
			const_cast<char* const*>(env.data()));
		std::exit(127);
	}

	this->input = pin[0];
	pipes[0] = pin[0];
	close(pin[1]);
	this->output = pout[1];
	pipes[1] = pout[1];
	close(pout[0]);
	std::signal(SIGCHLD, SIG_IGN);
	waitpid(this->pid, &this->status, WNOHANG);
	this->request = request;
	if (!this->status)
		log << Log::DEBUG << "Executing CGI - PID: " << this->pid << std::endl;
	return !this->status;
}

/* TODO:
 * NON_BLOCKING write to pipe
 * writing request body (un-chunked)
 * @return: true to keep alive, false to close
 */
bool CGIHandler::send_output()
{
	if (this->request->get_method() == Request::GET
	|| this->request->get_method() == Request::HEAD) {
		log << Log::DEBUG << "CGI GET method has no body - closing"
			<< std::endl;
		this->output = -1;
		return false;
	}
	if (this->request->is_chunked()) {
		if (!this->chunkster) {
			this->chunkster = new (std::nothrow) ChunkNorris;
			if (!this->chunkster) {
				this->request->set_status(500);
				return false;
			}
		}
		if (!this->chunkster->is_done()
		&& !this->request->is_body_loaded()
		&& !this->chunkster->nunchunkMe(this->request))
			return false;
		if (!this->chunkster->is_done())
			return true;
		this->wbuf.insert(
			this->wbuf.end(),
			this->chunkster->get_unchunked()->begin(),
			this->chunkster->get_unchunked()->end()
		);
	} else {
		size_t	body_size(utils::str_tonum<size_t>(
				this->request->get_header("content-length")));
		
		log << Log::DEBUG << "Loading request body" << std::endl;
		(void)this->request->load_payload(body_size);

		if (this->wbuf.empty()) {
			this->wbuf.reserve(this->request->get_loaded_body_size());
			this->wbuf = this->request->get_payload();
		}
	}
	log << Log::DEBUG << "CGI output to pipe" << std::endl;
	ssize_t s = write(this->output, this->wbuf.data(), this->wbuf.size());
	log << Log::DEBUG << "CGIHandler Transfered " << s
		<< " bytes of request body" << std::endl;
	if (s < 0)
		return false;
	if (!s)
		return true;
	if (static_cast<size_t>(s) < this->wbuf.size()) {
		this->wbuf.erase(this->wbuf.begin(), this->wbuf.begin() + s);
		return true;
	}
	this->wbuf.clear();
	this->output = -1;
	return false;
}

/* TODO:
 * NON_BLOCKING read from pipe
 * Use internal buffer
 * apply headers as needed,
 * after empty line begins message body for Content-Length bytes
 * prepend with status line before sending to client
 * @return: true to keep alive, false to close
 */
bool CGIHandler::read_input()
{
	char	buff[4096 * 2];

	log << Log::DEBUG << "CGI input from pipe" << std::endl;
	ssize_t r = read(this->input, buff, sizeof(buff));
	log << Log::DEBUG << "CGIHandler Read " << r
		<< " bytes of CGI reply" << std::endl;
	if (r < 1) {
		if (this->rbuf.empty()) {
			this->request->set_status(502);
			this->reply_error();
		} else
			this->prepare_reply();
		this->owner->switch_epoll_mode(this->client_fd, EPOLLOUT);
		return false;
	}
	this->rbuf.insert(this->rbuf.end(), buff, buff + r);
	buff[r] = 0;
	log << Log::DEBUG << ">>\n" << buff << std::endl;
	return true;
}

void CGIHandler::on_pipe_close(int fd)
{
	if (this->request->get_status() >= 400) {
		this->owner->switch_epoll_mode(this->client_fd, EPOLLOUT);
		this->reply_error();
		return;
	}
	if (fd == this->input) {
		this->owner->switch_epoll_mode(this->client_fd, EPOLLOUT);
		if (this->reply.empty()) {
			if (this->rbuf.empty()) {
				this->request->set_status(502);
				this->reply_error();
			} else
				this->prepare_reply();
		}
	}
}

/* TODO:
 * NON_BLOCKING send queued reply to client
 * @return: true to keep alive, false to close
 */
bool CGIHandler::send_reply()
{
	log << Log::DEBUG << "CGI sending reply" << std::endl;
	if (reply.empty()) {
		// this->read_input();
		log << Log::DEBUG << "CGI reply empty" << std::endl;
		return true;
	}
	ssize_t s = send(client_fd, this->reply.data(), this->reply.size(), MSG_DONTWAIT);
	log << Log::DEBUG << "CGI sent " << s << " bytes reply" << std::endl;
	if (s < 0)
		return false;
	if (!s)
		return true;
	if (static_cast<size_t>(s) < this->reply.size()) {
		this->reply.erase(this->reply.begin(), this->reply.begin() + s);
		return true;
	}
	this->reply.clear();
	return false;
}

/* TODO:
 * NON_BLOCKING recv from client, keep doing while !request->is_body_loaded()
 * @return: true to keep alive, false to close
 */
bool CGIHandler::receive()
{
	char	buff[4096];

	ssize_t r = recv(this->client_fd, buff, sizeof(buff), MSG_DONTWAIT);
	log << Log::DEBUG << "CGI received " << r << " bytes of body" << std::endl;
	switch (r) {
		case -1:
			return false;
		case 0:
			log << Log::INFO << "Client closed connection" << std::endl;
			return false;
		default:
			this->wbuf.insert(this->wbuf.end(), buff, buff + r);
	}
	if (this->request->is_chunked() && this->chunkster) {
		if (!this->chunkster->nunchunkMe(this->wbuf)) {
			this->owner->switch_epoll_mode(this->client_fd, EPOLLOUT);
			this->request->set_status(400);
			this->reply_error();
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

void CGIHandler::prepare_reply()
{
	this->read_headers();
	if (this->request->get_status() >= 400) {
		this->reply_error();
		return;
	}
	Reply::generate_response(
		this->request,
		this->headers,
		this->rbuf,
		this->reply);
	log << Log::DEBUG << "CGI prepared response " << this->reply.size()
		<< " bytes" << std::endl;
}

void CGIHandler::read_headers()
{
	std::vector<char>::iterator	c(this->rbuf.begin());
	while (c != this->rbuf.end()) {
		std::vector<char>::iterator	begin = c;
		while (c != this->rbuf.end() && *c++ != '\n')
			;
		if (c == this->rbuf.end()) {
			this->rbuf.erase(this->rbuf.begin(), begin);
			return;
		}
		std::string	tmp(begin, c);
		tmp = utils::trim(tmp, " \t\v\r\n");
		if (tmp.empty())
			break;
		std::string::size_type	div(tmp.find(":"));
		if (div == std::string::npos) {
			log << Log::WARN << "Invalid header received from CGI! Omitting..."
				<< std::endl;
			continue;
		}
		std::string	key(tmp.substr(0, div));
		std::string	val(tmp.substr(div + 1));
		if (utils::icompare(key, "status")) {
			int	status(utils::str_tonum<int>(val));
			if (status < 100 || status > 599) {
				log << Log::DEBUG << "Bad status code received from CGI"
					<< std::endl;
				this->rbuf.clear();
				this->request->set_status(502);
				this->reply_error();
				return;
			}
			this->request->set_status(status);
		} else {
			// TODO: restrict to allowed headers only
			this->headers.set_header(key, val);
		}
	}
	// TODO: ensure presence of headers required by protocol
	// TODO: handle error responses
	if (this->request->get_status() >= 400){
		;}
	std::string	len(this->headers.get_header("Content-Length"));
	size_t		body_size(0);
	if (!len.empty())
		body_size = utils::str_tonum<size_t>(len);
	if (body_size) {
		this->rbuf.erase(this->rbuf.begin(), c);
		if (this->rbuf.size() > body_size)
			this->rbuf.erase(this->rbuf.begin() + body_size, this->rbuf.end());
	} else
		this->rbuf.clear();
}

void CGIHandler::reply_error()
{
	std::vector<char>	body;
	this->owner->prepare_error_page(this->request, this->headers, body);
	Reply::generate_response(
		this->request,
		this->headers,
		body,
		this->reply);
	log << Log::DEBUG << "CGI prepared error response " << this->reply.size()
		<< " bytes" << std::endl;
}
