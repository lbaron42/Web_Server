#include <cstdlib>
#include <fstream>
#include <string>

#include "Headers.hpp"
#include "Log.hpp"

namespace {
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

static bool test_set(Headers &h, std::string const &key, std::string const &val);
static bool test_unset(Headers &h, std::string const &key);
static bool test_is_set(Headers &h, std::string const &key, std::string const &val);
static bool test_is_unset(Headers &h, std::string const &key);

int main()
{
	Headers		test;
	Log			log;

	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	if (!test_set(test, "Content-Type", "text/html; charset=utf-8")
	|| !test_set(test, "Connection", "")
	|| !test_set(test, "Connection", "Keep-Alive")
	|| !test_set(test, "", "")
	|| !test_set(test, "", "empty-value")
	|| !test_set(test, "empty-key", "")
	|| !test_set(test, "Content-Type", "text/html")
	|| !test_set(test, "Content-Encoding", "")
	|| !test_set(test, "Unset-Header", "Unset this value please")
	|| !test_set(test, "Keep-Alive", "timeout=5, max=42"))
		return EXIT_FAILURE;

	if (!test_unset(test, "empty-key")
	|| !test_unset(test, "Connection")
	|| !test_unset(test, "Unset-Header")
	|| !test_unset(test, ""))
		return EXIT_FAILURE;

	if (!test_is_set(test, "Connection", "Close")
	|| !test_is_set(test, "X-Frame-Options", "DENY")
	|| !test_is_set(test, "Date", "Sun, 19 May 2024 15:05:58 GMT")
	|| !test_is_set(test, "Expires", "Sun, 19 May 2024 15:56:36 GMT"))
		return EXIT_FAILURE;

	if (!test_is_unset(test, "Content-Encoding")
	|| !test_is_unset(test, "X-Frame-Options")
	|| !test_is_unset(test, "Unset-Header")
	|| !test_is_unset(test, ""))
		return EXIT_FAILURE;

	log << Log::DEBUG << "Test headers: " << std::endl
		<< test << std::endl;
	return EXIT_SUCCESS;
}

bool test_set(Headers &h, std::string const &key, std::string const &val)
{
	h.set_header(key, val);
	return (h.get_header(key) == val);
}

bool test_unset(Headers &h, std::string const &key)
{
	h.set_header(key, "Test value");
	h.unset_header(key);
	return h.get_header(key).empty();
}

bool test_is_set(Headers &h, std::string const &key, std::string const &val)
{
	h.set_header(key, val);
	return h.is_set(key);
}

bool test_is_unset(Headers &h, std::string const &key)
{
	h.set_header(key, "Test value");
	h.unset_header(key);
	return (!h.is_set(key));
}
