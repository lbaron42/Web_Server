/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 07:54:45 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/22 12:57:27 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Reply.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Static methods
////////////////////////////////////////////////////////////////////////////////

std::string const Reply::get_content(std::string const &filename)
{
	std::ifstream	file(filename.c_str());
	std::string		content;

	if (!file.is_open())
		return std::string();
	std::getline(file, content, std::string::traits_type::to_char_type(
			std::string::traits_type::eof()));
	return content;
}

std::vector<char> const Reply::get_payload(std::string const &filename)
{
	std::ifstream		file(filename.c_str(), std::ios::binary);
	std::vector<char>	bytes;

	ssize_t file_size = get_file_size(filename);
	if (file_size < 0)
		return bytes;
	bytes.reserve(file_size);
	bytes.insert(bytes.begin(), std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>());
	return bytes;
}

// TODO: 
std::string const Reply::get_listing(std::string const &path)
{
	std::stringstream	html;

	html << "<html>" << std::endl << "<head><title>"
		<< "Index of " << path << "</title></head>" << std::endl
		<< "<body>" << std::endl
		<< "<h1>Index of " << path 
		<< "</h1><hr><pre><a href=\"../\">..</a>" << std::endl
		<< "<h2>TODO</h2>" << std::endl
		<< "</body>" << std::endl << "</html>" << std::endl;
	return html.str();
}

std::string const Reply::get_status_line(bool v11, int status)
{
	std::ostringstream	oss;
	if (v11)	oss << "HTTP/1.1 ";
	else		oss << "HTTP/1.0 ";
	oss << status << " " << Reply::get_status_message(status) << "\r\n";
	return oss.str();
}

// TODO:
std::string const Reply::generate_error_page(int status)
{
	std::stringstream	html;

	html << "<html>" << std::endl
		<< "<head><title>"
		<< status << " - " << Reply::get_status_message(status)
		<< "</title></head>" << std::endl
		<< "<body>" << std::endl
		<< "<h1>" << "Error " << status
		<< "</h1><hr><pre>" << "<h2>" << Reply::get_status_message(status)
		<< "</h2>" << std::endl
		<< "</body>" << std::endl << "</html>" << std::endl;
	return html.str();
}

size_t Reply::get_html_size(int status)
{
	return (Reply::generate_error_page(status)).length();
}

size_t Reply::get_html_size(std::string const &listed_directory)
{
	return (Reply::get_listing(listed_directory)).length();
}

std::string const Reply::get_status_message(int status)
{
	switch (status) {
		// 1xx Information responses
		case 100:
			return "Continue";
		case 101:
			return "Switching Protocols";
		case 102:
			return "Processing";
		case 103:
			return "Early Hints";
		// 2xx Succesful responses
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-Authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 207:
			return "Multi-Status";
		case 208:
			return "Already Reported";
		case 226:
			return "IM Used";
		// 3xx Redirection messages
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		// case 305: return "Use Proxy";  // DEPRECATED
		// case 306: return "";  // no longer unused, but reserved
		case 307:
			return "Temporary Redirect";
		case 308:
			return "Permanent Redirect";
		// 4xx Client error responses
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 402:
			return "Payment Required";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition Failed";
		case 413:
			return "Content Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Range Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 418:
			return "I'm a teapot";
		case 421:
			return "Misdirected Request";
		case 422:
			return "Unprocessable Content";
		case 423:
			return "Locked";
		case 424:
			return "Failed Dependency";
		case 425:
			return "Too Early";
		case 426:
			return "Upgrade Required";
		case 428:
			return "Precondition Required";
		case 429:
			return "Too Many Requests";
		case 431:
			return "Request Header Fields Too Large";
		case 451:
			return "Unavailable For Legal Reasons";
		// 5xx Server error responses
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		case 506:
			return "Variant Also Negotiates";
		case 507:
			return "Insufficient Storage";
		case 508:
			return "Loop Detected";
		case 510:
			return "Not Extended";
		case 511:
			return "Network Authentication Required";
		default:
			return std::string();
	}
}
