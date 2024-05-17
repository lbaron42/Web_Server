#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>

#include "Log.hpp"

static bool test_std_streams();
static bool test_file_write();

namespace {
	static char const	*test_file = "test.log";
}

int main(void)
{
	if (!test_std_streams() || !test_file_write())
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

bool test_std_streams()
{
	Log log;

	log << "is std::cout?" << std::endl;
	log.set_output(&std::cerr, false);
	log << "is std::cerr?" << std::endl;
	return true;
}

bool test_file_write()
{
	Log log;

	log.set_output(new std::ofstream(test_file, \
		std::ios_base::ate | std::ios_base::ate), true);
	log << "Opened the file" << std::endl
		<< "In appending mode" << std::endl
		<< " for each write call" << std::endl;
	log << "Write another line" << std::endl;
	log.set_output(0, false);
	std::remove(test_file);
	return true;
}
