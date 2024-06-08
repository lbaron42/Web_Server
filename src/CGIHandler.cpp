/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 17:57:12 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/08 04:54:16 by mcutura          ###   ########.fr       */
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
		owner(owner)
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
		owner(src.owner)
{}

/* TODO
 * close pipes, terminate child process, delete request
 */
CGIHandler::~CGIHandler()
{
	close(this->input);
	close(this->output);
	if (this->pid > 1)
		kill(this->pid, SIGKILL);
	if (this->request)
		delete this->request;
}

////////////////////////////////////////////////////////////////////////////////
//	Signal handling
////////////////////////////////////////////////////////////////////////////////

extern "C" void on_cgi_exit(int sig)
{
	if (sig != SIGCHLD)
		return;
	waitpid(-1, NULL, WNOHANG);
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
bool CGIHandler::execute(int pipes[2], Request *request)
{
	std::vector<char const *>	argv;
	std::string					cmd(request->get_target());
	argv.push_back(cmd.c_str());
	argv.push_back(NULL);

	std::vector<char const *> env(request->get_headers().get_as_env());
	std::string path("PATH_INFO=");
	// path.append(); // TODO: which path to pass?
	env.push_back(path.c_str());
	std::string method("REQUEST_METHOD=");
	method.append(request->get_method_as_str());
	env.push_back(method.c_str());
	std::string query("QUERY_STRING=");
	query.append(request->get_query());
	env.push_back(query.c_str());
	std::string gateway("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back(gateway.c_str());
	env.push_back(NULL);

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
	std::signal(SIGCHLD, on_cgi_exit);
	waitpid(this->pid, &this->status, WNOHANG);
	this->request = request;
	if (!this->status)
		log << Log::DEBUG << "Executing CGI - PID: " << this->pid << std::endl;

	// log << Log::DEBUG << "Testing pipes" << std::endl;
	// while (read(this->input, this->rbuf.data(), 4096) > 0) {
	// 	log << this->rbuf.data() << std::endl;
	// }
	return !this->status;
}

/* TODO:
 * NON_BLOCKING write to pipe
 * writing request body (un-chunked)
 * @return: true to keep alive, false to close
 */
bool CGIHandler::send_output()
{
	if (request->get_method() == Request::GET) {
		log << Log::DEBUG << "CGI GET method has no body" << std::endl;
		// close(this->output);
		this->output = -1;
		return false;
	}
	size_t	body_size(str_tonum<size_t>(
			this->request->get_header("content-length")));
	if (!this->request->is_body_loaded()) {
		log << Log::DEBUG << "Loading request body" << std::endl;
		if (this->request->load_payload(body_size))
			return true;
	}
	if (this->wbuf.empty()) {
		this->wbuf.reserve(this->request->get_loaded_body_size());
		this->wbuf = this->request->get_payload();
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
	// close(this->output);
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
		if (this->reply.empty())
			this->reply_error(502);
		else
			this->prepare_reply();
		this->owner->switch_epoll_mode(this->client_fd, EPOLLOUT);
		return false;
	}
	this->rbuf.insert(this->rbuf.end(), buff, buff + r);
	return true;
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
			close(this->client_fd);
			return false;
		case 0:
			log << Log::INFO << "Client closed connection" << std::endl;
			close(this->client_fd);
			return false;
		default:
			this->wbuf.insert(this->wbuf.end(), buff, buff + r);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////

void CGIHandler::prepare_reply()
{
	std::vector<char>::iterator	c(this->rbuf.begin());
	while (c != this->rbuf.end()) {
		std::vector<char>::iterator	begin = c;
		while (c != this->rbuf.end() && *c++ != '\n')
			;
		std::string	tmp(begin, c);
		tmp = trim(tmp, " \t\v\r\n");
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
		if (icompare(key, "status")) {
			int	status(str_tonum<int>(val));
			if (status < 100 || status > 599) {
				log << Log::DEBUG << "Bad status code received from CGI"
					<< std::endl;
				this->rbuf.clear();
				this->reply_error(502);
				return;
			}
			this->request->set_status(status);
		} else {
			// TODO: restrict to allowed headers only
			this->headers.set_header(key, val);
		}
	}
	std::string	len(this->headers.get_header("Content-Length"));
	size_t		body_size(0);
	if (!len.empty())
		body_size = str_tonum<size_t>(len);
	if (body_size) {
		this->rbuf.erase(this->rbuf.begin(), c);
		if (this->rbuf.size() > body_size)
			this->rbuf.erase(this->rbuf.begin() + body_size, this->rbuf.end());
	} else
		this->rbuf.clear();
	Reply::generate_response(
		this->request,
		this->headers,
		this->rbuf,
		this->reply);
	log << Log::DEBUG << "CGI prepared response " << this->reply.size()
		<< " bytes" << std::endl;
}

void CGIHandler::reply_error(int status)
{
	this->request->set_status(status);
	std::string	tmp(Reply::generate_error_page(status));
	std::vector<char>	body;
	body.reserve(tmp.size());
	body.insert(body.end(), tmp.begin(), tmp.end());
	this->headers.set_header("Content-Length", num_tostr(body.size()));
	this->headers.set_header("Content-Type", "text/html");
	this->headers.set_header("Connection", "close");
	Reply::generate_response(
		this->request,
		this->headers,
		body,
		this->reply);
	log << Log::DEBUG << "CGI prepared response " << this->reply.size()
		<< " bytes" << std::endl;
}
