/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 08:22:55 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/15 09:48:12 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

namespace {
	static char const *const	methodnames[] = {
		"NONE", "HEAD", "GET", "POST", "PUT", "PATCH",
	"DELETE", "OPTIONS", "CONNECT", "TRACE" };
	static size_t const			n_methods = 10;
}

////////////////////////////////////////////////////////////////////////////////
//	CTOR/DTOR
////////////////////////////////////////////////////////////////////////////////

Request::Request(std::string const &raw, Log &logger)
	:	log(logger),
		raw_(raw, std::ios_base::in | std::ios_base::out
			| std::ios_base::ate | std::ios_base::binary),
		req_line(),
		method(Request::NONE),
		url(),
		query(),
		v_11(true),
		is_parsed_(false),
		is_dirlist_(false),
		status(0),
		headers(),
		target(),
		script(),
		path(),
		loaded_body_size(0),
		is_body_loaded_(false),
		payload(),
		filenames(),
		bounced(false),
		is_chunked_(false)
{}

Request::Request(Request const &rhs)
	:	log(rhs.log),
		raw_(rhs.raw_.str(), std::ios_base::in | std::ios_base::out
			| std::ios_base::ate | std::ios_base::binary),
		req_line(rhs.req_line),
		method(rhs.method),
		url(rhs.url),
		query(rhs.query),
		v_11(rhs.v_11),
		is_parsed_(rhs.is_parsed_),
		is_dirlist_(rhs.is_dirlist_),
		status(rhs.status),
		headers(rhs.headers),
		target(rhs.target),
		script(rhs.script),
		path(rhs.path),
		loaded_body_size(rhs.loaded_body_size),
		is_body_loaded_(rhs.is_body_loaded_),
		payload(rhs.payload),
		filenames(rhs.filenames),
		bounced(rhs.bounced),
		is_chunked_(rhs.is_chunked_)
{}

Request::~Request()
{
	this->payload.clear();
	this->filenames.clear();
}

////////////////////////////////////////////////////////////////////////////////
//	Getters/Setters
////////////////////////////////////////////////////////////////////////////////

Request::e_method Request::get_method() const
{
	return this->method;
}

std::string Request::get_method_as_str() const
{
	unsigned int m = static_cast<unsigned int>(this->method);
	unsigned int c = 0;
	if (m)
		while (++c < n_methods && !(m & 1))
			m >>= 1;
	return std::string(methodnames[c]);
}

std::string Request::get_url() const
{
	return this->url;
}

std::string Request::get_query() const
{
	return this->query;
}

bool Request::is_version_11() const
{
	return this->v_11;
}

std::string const Request::get_header(std::string const &key) const
{
	return this->headers.get_header(key);
}

Headers Request::get_headers() const
{
	return this->headers;
}

std::string Request::get_req_line() const
{
	return this->req_line;
}

int Request::get_status() const
{
	return this->status;
}

std::string Request::get_target() const
{
	return this->target;
}

std::string Request::get_script() const
{
	return this->script;
}

std::string Request::get_path() const
{
	return this->path;
}

std::vector<char> Request::get_payload() const
{
	return this->payload;
}

std::vector<std::string> Request::get_filenames() const
{
	return this->filenames;
}

bool Request::is_dirlist() const
{
	return this->is_dirlist_;
}

bool Request::is_parsed() const
{
	return this->is_parsed_;
}

// TODO: rewrite
bool Request::is_done() const
{
	return (this->raw_.eof());
}

bool Request::is_bounced() const
{
	return this->bounced;
}

bool Request::is_chunked() const
{
	return this->is_chunked_;;
}

size_t Request::get_loaded_body_size() const
{
	return this->loaded_body_size;
}

bool Request::is_body_loaded() const
{
	return this->is_body_loaded_;
}

void Request::set_status(int status)
{
	this->status = status;
}

void Request::set_target(std::string const &path)
{
	this->target = path;
}

void Request::set_script(std::string const &script)
{
	this->script = script;
}

void Request::set_path(std::string const &path)
{
	this->path = path;
}

void Request::set_dirlist(bool value)
{
	this->is_dirlist_ = value;
}

void Request::set_parsed(bool value)
{
	this->is_parsed_ = value;
}

void Request::set_bounced(bool value)
{
	this->bounced = value;
}

////////////////////////////////////////////////////////////////////////////////
//	Static methods
////////////////////////////////////////////////////////////////////////////////

Request::e_method Request::parse_methods(std::string const &str)
{
	std::stringstream	ss(str);
	std::string			method;
	unsigned int		allowed_methods(0);

	while (ss >> method) {
		for (size_t i = 1; i < n_methods; ++i) {
			if (!method.compare(methodnames[i])) {
				allowed_methods |= (1 << --i);
				break;
			}
		}
	}
	return static_cast<Request::e_method>(allowed_methods);
}

////////////////////////////////////////////////////////////////////////////////
//	Public methods
////////////////////////////////////////////////////////////////////////////////

void Request::append(std::string const &str)
{
	this->raw_.clear();
	this->raw_ << str;
}

bool Request::validate_url()
{
	if (this->url.rfind("http://", 0) == std::string::npos)
		return true;
	std::string::size_type	pos(this->url.find_first_of('/', 7));
	if (pos == std::string::npos)
		this->url = "/";
	else
		this->url = this->url.substr(pos);
	return true;
}

int Request::validate_request_line()
{
	if (utils::size_of_stream(this->raw_) < 15)
		return 0;
	if (utils::size_of_stream(this->raw_) > 4096 * 2)
		return 414;
	while (std::getline(this->raw_, this->req_line)) {
		if (this->req_line.empty()
		|| (this->req_line.length() == 1 && this->req_line == "\r"))
			continue;
		if (this->raw_.eof()) {
			this->raw_.seekg(0);
			return 0;
		}
		log << Log::DEBUG << "Request line: " << this->req_line << std::endl;

		std::string::size_type first(this->req_line.find(' '));
		if (first == std::string::npos)
			return 400;
		if (first < 3 || first > 6
		|| !this->is_valid_method(this->req_line.substr(0, first)))
			return 400;
		log << Log::DEBUG << "Method:		["
			<< this->req_line.substr(0, first) << "]"
			<< std::endl;

		std::string::size_type second(this->req_line.find(' ', first + 1));
		if (second == std::string::npos || second == first + 1)
			return 400;
		this->url = this->req_line.substr(first + 1, second - first - 1);
		if (!utils::is_valid_url(this->url)) {
			log << Log::DEBUG << "URL not valid" << std::endl;
			return 400;
		}
		std::string::size_type q(this->url.find('?'));
		if (!q)
			return 400;
		if (q != std::string::npos) {
			this->query = this->url.substr(q + 1);
			this->url.erase(q);
		}
		this->url = utils::url_decode(this->url);
		if (!validate_url())
			return 400;
		log << Log::DEBUG << "URL:		[" << this->url << "]" << std::endl;
		if (!this->query.empty()) {
			log << Log::DEBUG << "Query		[" << this->query << "]"
				<< std::endl;
		}
		if (this->url.empty()) {
			log << Log::DEBUG << "Requested URL not valid" << std::endl;
			return 400;
		}

		std::string::size_type end(this->req_line.find('\r', second + 1));
		if (end == std::string::npos) {
			log << Log::DEBUG
				<< "Request line terminated by '\\n' instead of '\\r\\n'"
				<< std::endl;
		} else if (end != second + 9 || end != this->req_line.length() - 1)
			return 400;
		this->req_line = utils::trim(this->req_line, "\r");
		log << Log::DEBUG << "Version:	["
			<< this->req_line.substr(second + 1, end - second - 1)
			<< "]" << std::endl;
		std::string vers(this->req_line.substr(second + 1, end - second - 1));
		if (vers == "HTTP/1.1") {
			this->v_11 = true;
			return 200;
		} else if (vers == "HTTP/1.0") {
			this->v_11 = false;
			return 200;
		} else
			return 505;
	}
	return 400;
}

bool Request::is_valid_method(std::string const &method)
{
	for (size_t i = 1; i < n_methods; ++i) {
		if (!method.compare(methodnames[i])) {
			this->method = static_cast<Request::e_method>(1 << --i);
			return true;
		}
	}
	log << Log::DEBUG << method << " is not a valid method" << std::endl;
	return false;
}

bool Request::parse_headers()
{
	std::string	header;

	while (std::getline(this->raw_, header)) {
		if (this->raw_.eof()) {
			if (header.empty())
				return true;
			this->raw_.clear();
			this->raw_.str(std::string());
			this->raw_ << header;
			log << Log::DEBUG << "Partial header returned to raw stream"
				<< std::endl;
			return false;
		}
		if (header.empty() || header == "\r") {
			return true;
		}
		std::string::size_type div = header.find(':');
		if (div == std::string::npos) {
			log << Log::WARN << "Received malformatted header:" << std::endl
				<< header << std::endl;
			continue;
		}
		std::string key = utils::trim(header.substr(0, div));
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::string val = utils::trim(header.substr(div + 1), " \t\r");
		// TODO: manage cookies
		if (utils::icompare(key, "Cookie")) {;}
		this->headers.set_header(key, val);
		if (utils::icompare(key, "Transfer-Encoding")
		&& utils::icompare(val, "chunked"))
			this->is_chunked_ = true;
	}
	return false;
}

std::string Request::load_body()
{
	std::stringstream	ss;
	ss << this->raw_.rdbuf();
	this->is_body_loaded_ = true;
	this->raw_.clear();
	this->raw_.str(std::string());
	return ss.str();
}

// TODO: test and validate
bool Request::load_payload(size_t size)
{
	if (!this->loaded_body_size)
		this->payload.reserve(size);
	std::streampos	inpos = this->raw_.tellg();
	std::streampos	outpos = this->raw_.tellp();

	bool partial(static_cast<std::streampos>(size) > (outpos - inpos));
	log << Log::DEBUG << "Partial: " << partial << std::endl
		<< "Inpos: " << inpos << " | Outpos: " << outpos << std::endl
		<< "Size: " << size << std::endl
		<< "EOF: " << this->raw_.eof() << std::endl;
	std::string	tmp(this->load_body());
	log << Log::DEBUG << "Body:\n" << tmp << std::endl;
	this->payload.insert(this->payload.end(), tmp.begin(), tmp.end());
	this->loaded_body_size += partial	? static_cast<size_t>(outpos - inpos)
										: size;
	this->is_body_loaded_ = !partial;
	return partial;
}

void Request::drop_payload()
{
	this->payload.clear();
}

bool Request::load_multipart(std::string const &boundary, size_t max_body_size)
{
	std::string				line;
	std::string::size_type	div;

	if (this->payload.empty()) {
		this->headers.unset_header("content-disposition");
		if (std::getline(this->raw_, line)) {
			this->loaded_body_size += line.length() + 1;
			if (line == boundary + "--\r"
			|| line == boundary + "--") {
				this->is_body_loaded_ = true;
				log << Log::DEBUG << "END multipart body" << std::endl;
				return true;
			}
			if (line != boundary + "\r" && line != boundary) {
				log << Log::DEBUG << "Reject - body not starting with boundary"
					<< std::endl << "[" << line << "]" << std::endl;
				this->status = 400;
				return true;
			}
			log << Log::DEBUG << "Found start boundary" << std::endl;
		} else {
			log << Log::WARN << "Raw stream EOF" << std::endl;
			// this->status = 400;
			return true;
		}
		while (std::getline(this->raw_, line)) {
			this->loaded_body_size += line.length() + 1;
			if (line.empty() || line == "\r")
				break;
			div = line.find(":");
			if (div == std::string::npos) {
				log << Log::WARN << "No header divider" << std::endl;
				this->status = 400;
				return true;
			}
			std::string	key(utils::trim(line.substr(0, div)));
			std::string	val(utils::trim(line.substr(div + 1), " \t\r"));
			if (key.empty() || val.empty()) {
				log << Log::WARN << "Empty header key/value" << std::endl;
				this->status = 400;
				return true;
			}
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			// TODO: Feature - Could check for forbidden mime types here
			if (key == "content-disposition")
				this->headers.set_header(key, val);
		}
		std::string	disp = this->headers.get_header("content-disposition");
		if (disp.empty()) {
			log << Log::WARN << "No content disposition" << std::endl;
			this->status = 400;
			return true;
		}
		log << Log::DEBUG << "Disposition:	[" << disp << "]" << std::endl;
		div = disp.find("filename=");
		if (div == std::string::npos) {
			log << Log::WARN << "No disposition filename" << std::endl;
			this->status = 400;
			return true;
		}
		std::string::size_type end(disp.find("\"", div + 10));
		if (end == std::string::npos) {
			log << Log::WARN << "Unclosed quote around filename" << std::endl;
			this->status = 400;
			return true;
		}
		std::string const filename(utils::trim(
			disp.substr(div + 10, end - div - 10), "\""));
		if (filename.empty()) {
			log << Log::DEBUG << "Empty file upload / no filename"
				<< std::endl;
			this->status = 400;
			return true;
		}
		this->path = this->target + filename;
		log << Log::DEBUG << "Filepath: [" << this->path << "]" << std::endl;
	}
	std::string	part = utils::get_delimited(this->raw_, boundary);
	if (part.empty()) {
		log << Log::DEBUG << "EMPTY delimited. Tellg: " << this->raw_.tellg()
			<< std::endl;
		this->raw_.clear();
	}
	this->loaded_body_size += part.length();
	if (this->loaded_body_size > max_body_size) {
		this->status = 413;
		return true;
	}
	log << Log::DEBUG << "Delimited:" << std::endl << "["
		<< part << "]" << std::endl << "[" << boundary << "]" << std::endl;
	div = part.find(boundary);
	if (div == std::string::npos) {
		log << Log::DEBUG << "No end file boundary found" << std::endl;
		this->payload.insert(this->payload.end(), part.begin(), part.end());
		return false;
	}
	std::string	next(part.substr(div));
	this->loaded_body_size -= next.length();
	log << Log::DEBUG << "Next [" << next << "]" << std::endl;
	part.erase(div - 1);
	this->payload.insert(this->payload.end(), part.begin(), part.end());
	if (!utils::save_file(this->path, this->payload)) {
		log << Log::ERROR << "Error saving file" << std::endl;
		this->status = 500;
		return true;
	}
	this->filenames.push_back(this->path.substr(this->target.size()));
	this->payload.clear();
	std::string	tmp(this->raw_.str().substr(this->raw_.tellg()));
	this->raw_.str(std::string());
	this->raw_.clear();
	this->raw_ << next << tmp;
	return this->load_multipart(boundary, max_body_size);
}
