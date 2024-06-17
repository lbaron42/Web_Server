#include <cstdlib>

#include "Server.hpp"
#include "Log.hpp"

namespace {
	Log					log;
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

int main()
{

	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	// TODO: Add unit tests here

	return EXIT_SUCCESS;
}
