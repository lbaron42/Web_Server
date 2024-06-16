#include <cstdlib>
#include <string>

#include "ChunkNorris.hpp"
#include "Log.hpp"
#include "Request.hpp"

namespace {
	Log					log;
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

static std::string	create_test_sample();
static Request *create_mock_request();
static bool test_nunchunkMe(Request *request, std::string const &expected);

int main()
{
	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	std::string	test_sample(create_test_sample());
	Request	*request(create_mock_request());
	if (test_nunchunkMe(request, test_sample))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

std::string	create_test_sample()
{
	std::stringstream raw;
	raw << "A"
		<< "abcd"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901"
		<< "0123456789"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901"
		<< "0123456789012345678901234567890123456789012"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901"
		<< "01234567890123456789012345678901234567890"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901"
		<< "012345678901234567890123456789012345678"
		<< "012345678901234567890123456789012345678901"
		<< "01234567890123456789012345678901234567890123"
		<< "012345678901234567890123456789012345678901"
		<< "01234567890123456789012345678901"
		<< "012345678901234567890123456789012345678901";
	return raw.str();
}

Request *create_mock_request()
{
	std::stringstream raw;
	raw << "POST /test HTTP/1.1\r\n"
		<< "Transfer-Encoding: Chunked\r\n"
		<< "Content-Type: application/octet-stream\r\n"
		<< "Host: test.test\r\n\r\n"
		<< "1\r\n" << "A\r\n"
		<< "4\r\n" << "abcd\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "A\r\n" << "0123456789\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2B\r\n" << "0123456789012345678901234567890123456789012\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "29\r\n" << "01234567890123456789012345678901234567890\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "27\r\n" << "012345678901234567890123456789012345678\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "2C\r\n" << "01234567890123456789012345678901234567890123\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "20\r\n" << "01234567890123456789012345678901\r\n"
		<< "2A\r\n" << "012345678901234567890123456789012345678901\r\n"
		<< "0\r\n\r\n";
	Request *req = new Request(raw.str(), log);
	return req;
}

bool test_nunchunkMe(Request *request, std::string const &expected)
{
	ChunkNorris	ranger;
	while (ranger.nunchunkMe(request) && !ranger.is_done())
		;
	if (!ranger.is_done()) {
		log << Log::ERROR << "Chunker failed to unchunk request" << std::endl;
		return false;
	}
	std::string	result(
		ranger.get_unchunked()->begin(),
		ranger.get_unchunked()->end());
	if (result.compare(expected)) {
		log << Log::ERROR << "Failed test_nunchunkMe(Request)"
			<< std::endl << "Expected:	" << expected
			<< std::endl << "Result:	" << result
			<< std::endl;
		return false;
	}
	return true;
}
