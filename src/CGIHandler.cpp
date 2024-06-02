/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 17:57:12 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/02 13:26:12 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

////////////////////////////////////////////////////////////////////////////////
//	CTORs/DTOR
////////////////////////////////////////////////////////////////////////////////

CGIHandler::CGIHandler(Log &log, int client, size_t max_body_size)
	:	log(log),
		request(NULL),
		client_fd(client),
		input(-1),
		output(-1),
		pid(-1),
		status(0),
		buff(),
		headers(),
		reply(),
		max_body_size(max_body_size)
{}

CGIHandler::CGIHandler(const CGIHandler &src)
	:	log(src.log),
		request(src.request),
		client_fd(src.client_fd),
		input(src.input),
		output(src.output),
		pid(src.pid),
		status(src.status),
		buff(src.buff),
		headers(src.headers),
		reply(src.reply),
		max_body_size(src.max_body_size)
{}

/* TODO
 * close pipes, terminate child process, delete request
 */
CGIHandler::~CGIHandler()
{
	if (this->request)
		delete request;
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
	env.push_back(NULL);

	int pin[2], pout[2];
	if (pipe(pin) || pipe(pout))
		return false;
	int input = dup2(pin[0], pipes[0]);
	int output = dup2(pout[1], pipes[1]);
	if (input < 0 || output < 0) {
		close(pin[0]);
		close(pin[1]);
		close(pout[0]);
		close(pout[1]);
		return false;
	}

	// TODO: fork -> setup pipeline -> execve
	// execve(argv[0],
	// 	const_cast<char* const*>(argv.data()),
	// 	const_cast<char* const*>(env.data()));
	return false;

	waitpid(this->pid, &this->status, WNOHANG);
	this->request = request;
	return !this->status;
}

/* TODO:
 * NON_BLOCKING write to pipe
 * writing request body (un-chunked)
 * @return: true to keep alive, false to close
 */
bool CGIHandler::send_output()
{
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
	return false;
}

/* TODO:
 * NON_BLOCKING send queued reply to client
 * @return: true to keep alive, false to close
 */
bool CGIHandler::send_reply()
{
	return false;
}

/* TODO:
 * NON_BLOCKING recv from client, keep doing while !request->is_body_loaded()
 * @return: true to keep alive, false to close
 */
bool CGIHandler::receive()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//	Private innards
////////////////////////////////////////////////////////////////////////////////
