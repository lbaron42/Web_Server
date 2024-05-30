#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "Request.hpp"
#include "Log.hpp"
#include "Headers.hpp"

struct Expected
{
	Request::e_method	method;
	std::string			url;
	bool				v11;
	int					status;
	Headers				hdrs;
	std::string			target;
	bool				should_fail;
	Expected()
		:	method(Request::NONE),
			url(),
			v11(false),
			status(0),
			hdrs(),
			target(),
			should_fail(false)
	{}
	Expected(Request::e_method m, std::string const &url, bool v11, int status,
		Headers headers, std::string const &target,
		bool should_fail = false)
		:	method(m),
			url(url),
			v11(v11),
			status(status),
			hdrs(headers),
			target(target),
			should_fail(should_fail)
	{}
	Expected(Expected const &rhs)
		:	method(rhs.method),
			url(rhs.url),
			v11(rhs.v11),
			status(rhs.status),
			hdrs(rhs.hdrs),
			target(rhs.target),
			should_fail(rhs.should_fail)
	{}
	Expected &operator=(Expected const &rhs)
	{
		if (this == &rhs)	return *this;
		this->method = rhs.method;
		this->url = rhs.url;
		this->v11 = rhs.v11;
		this->status = rhs.status;
		this->hdrs = rhs.hdrs;
		this->method = rhs.method;
		this->should_fail = rhs.should_fail;
		return *this;
	}
};

static std::map<std::string, Expected> generate_test_data();
static bool test_status(int input);
static bool test_parse_method(std::string const &input,
		Request::e_method expect, bool should_fail = false);
static bool test_is_valid_method(std::string const &input, bool expect,
		bool should_fail = false);
static bool test_validate_request_line(std::string const &input, int expect,
		bool should_fail = false);
static bool test_get_request_line(std::string const &input);
static bool test_get_url(std::string const &input, std::string const &expect,
		bool should_fail);
static bool test_is_version_11(std::string const &input, bool expect);

namespace {
	Log					log;
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

int main()
{
	std::map<std::string, Expected>	test_data = generate_test_data();

	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	if (!test_status(0)
	|| !test_status(200)
	|| !test_status(400)
	|| !test_status(404)
	|| !test_status(500)
	|| !test_status(504)
	|| !test_status(418)
	|| !test_status(-42)
	|| !test_status(42)) {
		return EXIT_FAILURE;
	}

	if (!test_parse_method("NONE", Request::NONE)
	|| !test_parse_method("HEAD", Request::HEAD)
	|| !test_parse_method("GET", Request::GET)
	|| !test_parse_method("POST", Request::POST)
	|| !test_parse_method("PUT", Request::PUT)
	|| !test_parse_method("PATCH", Request::PATCH)
	|| !test_parse_method("DELETE", Request::DELETE)
	|| !test_parse_method("OPTIONS", Request::OPTIONS)
	|| !test_parse_method("CONNECT", Request::CONNECT)
	|| !test_parse_method("TRACE", Request::TRACE)
	|| !test_parse_method("HEAD GET OPTIONS",
			static_cast<Request::e_method>(
				Request::HEAD|Request::GET|Request::OPTIONS))
	|| !test_parse_method("PUT POST DELETE",
			static_cast<Request::e_method>(
				Request::PUT|Request::POST|Request::DELETE))
	|| !test_parse_method("GET HEAD",
			static_cast<Request::e_method>(Request::GET|Request::HEAD))
	|| !test_parse_method("GET SCHWIFTY",
			static_cast<Request::e_method>(Request::GET))
	|| !test_parse_method("WHAT? WHY? HOW TO PUT IT OUT?",
			static_cast<Request::e_method>(Request::PUT))
	|| !test_parse_method("WHAT OPTIONS WE GOT PATCH IT UP NONE",
			static_cast<Request::e_method>(Request::OPTIONS|Request::PATCH))
	|| !test_parse_method("MAYBE", Request::CONNECT, true)
	|| !test_parse_method("GET", Request::OPTIONS, true)) {
		return EXIT_FAILURE;
	}

	if (!test_is_valid_method("GET", true)
	|| !test_is_valid_method("HEAD", true)
	|| !test_is_valid_method("HALLO", false)
	|| !test_is_valid_method("HELLO", true, true)
	|| !test_is_valid_method("NOGOOD", false)
	|| !test_is_valid_method("BUENO", true, true)
	|| !test_is_valid_method("OPTIONS", true)
	|| !test_is_valid_method("PUT", true)
	|| !test_is_valid_method("POST", true)
	|| !test_is_valid_method("PATCH", true)
	|| !test_is_valid_method("POST", false, true)) {
		return EXIT_FAILURE;
	}

	for (std::map<std::string, Expected>::const_iterator it = test_data.begin();
	it != test_data.end(); ++it) {
		if (!test_get_request_line(it->first)
		|| !test_validate_request_line(it->first,
			it->second.status, it->second.should_fail)
		|| !test_get_url(it->first, it->second.url,
			it->second.should_fail)
		|| !test_is_version_11(it->first, it->second.v11)
			) {
			log << Log::ERROR << "Failed test case: " << std::endl
				<< it->first << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

bool test_status(int input)
{
	Request	request("", log);

	request.set_status(input);
	return (request.get_status() == input);
}

bool test_parse_method(std::string const &input, Request::e_method expect,
		bool should_fail)
{
	Request::e_method result = Request::parse_methods(input);
	if (result != expect) {
		if (should_fail)	return true;
		log << Log::ERROR << "Failed parse_method" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return !should_fail;
}

bool test_validate_request_line(std::string const &input, int expect,
		bool should_fail)
{
	Request	request(input, log);
	int		result = request.validate_request_line();
	if (result != expect) {
		if (should_fail)	return true;
		log << Log::ERROR << "Failed validate_request_line" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return true;
}

bool test_get_request_line(std::string const &input)
{
	Request	request(input, log);
	std::string	removed_new_line(input);

	std::string::size_type	pos = input.find("\n");
	if (pos != std::string::npos)
		removed_new_line.erase(pos);
	pos = input.find("\r");
	if (pos != std::string::npos)
		removed_new_line.erase(pos);
	if (!request.validate_request_line())
		return request.get_req_line().empty();
	std::string	result = request.get_req_line();
	pos = result.find("\r");
	if (pos != std::string::npos)
		result.erase(pos);
	if (result != removed_new_line) {
		log << Log::ERROR << "Failed get_req_line" << std::endl
			<< "Input:	[" << removed_new_line << "]" << std::endl
			<< "Result:	[" << request.get_req_line() << "]" << std::endl;
		return false;
	}
	return true;
}

bool test_is_valid_method(std::string const &input, bool expect,
		bool should_fail)
{
	Request	request(input, log);
	bool	result = request.is_valid_method(input);

	if (result != expect) {
		if (should_fail)	return true;
		log << Log::ERROR << "Failed is_valid_method" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return !should_fail;
}

bool test_get_url(std::string const &input, std::string const &expect,
		bool should_fail)
{
	Request	request(input, log);

	(void)request.validate_request_line();
	std::string	result = request.get_url();
	if (result != expect) {
		if (should_fail)	return true;
		log << Log::ERROR << "Failed get_url" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
	}
	return !should_fail;
}

bool test_is_version_11(std::string const &input, bool expect)
{
	Request	request(input, log);

	(void)request.validate_request_line();
	bool result = request.is_version_11();
	if (result != expect) {
		log << Log::ERROR << "Failed is_version_11" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return true;
}

bool test_headers()
{
	return true;
}

std::map<std::string, Expected> generate_test_data()
{
	std::map<std::string, Expected>	data;
	data["GET / HTT"] = Expected(Request::NONE, "", true, 0, Headers(), "");
	data["GET / HTTP/1.1\n"] = Expected(Request::GET, "/", true, 200, Headers(),
		"/");
	data["GET / HTTP/1.1\r\n"] = Expected(Request::GET, "/", true, 200, Headers(),
		"/");
	data["GET  /  HTTP/1.1\r\n"] = Expected(Request::GET, "", true, 400,
		Headers(), "/");
	data["GET /index.html HTTP/1.1\r\n"] = Expected(Request::GET, "/index.html",
		true, 200, Headers(), "/index.html");
	data["GET /index.html HTTP/1.0\r\n"] = Expected(Request::GET, "/index.html",
		false, 200, Headers(), "/index.html");
	data["HEAD /index.html HTTP/1.1\r\n"] = Expected(Request::HEAD,
		"/index.html", true, 200, Headers(), "/index.html");
	data["GET /img HTTP/1.1\r\n"] = Expected(Request::GET, "/img", true, 200,
		Headers(), "/img");
	data["GET /img HTTP/1.1\r\n"] = Expected(Request::GET, "img", true, 200,
		Headers(), "img", true);
	data["GET / HTTP/1.2\r\n"] = Expected(Request::GET, "/", true, 505,
		Headers(), "");
	data["BET / HTTP/1.1\r\n"] = Expected(Request::NONE, "", true, 400, Headers(),
		"");
	return data;
}
