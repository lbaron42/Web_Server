#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>

#include "Log.hpp"

static bool test_file_write();
static bool check_content(char const *path, std::stringstream &expect);

namespace {
	static char const	*tests_log = "tests/unit_tests/tests.log";
	static char const	*test_file = "tests/unit_tests/test_file.log";
}

int main(void)
{
	std::ofstream		logfile(tests_log, std::ios_base::app);

	if (!logfile.is_open())
		return EXIT_FAILURE;
	if (!test_file_write())
	{
		logfile << "File test failed" << std::endl;
		return EXIT_FAILURE;
	}
	std::remove(test_file);
	return EXIT_SUCCESS;
}

bool check_content(char const *path, std::stringstream &expect)
{
	std::ofstream		logfile(tests_log);
	std::fstream		file(path, std::ios_base::in);
	std::string			fileline;
	std::string			expectline;

	if (!logfile.is_open())
	{
		std::cerr << "Couldn't open logfile" << std::endl;
		return false;
	}
	if (!file.is_open())
	{
		logfile << "Failed to open file " << path << std::endl;
		return false;
	}
	while(std::getline(file, fileline, '\n')
	&& std::getline(expect, expectline, '\n'))
	{
		if (fileline != expectline)
		{
			logfile << "Expected:	" << expectline << std::endl
				<< "Found:   	" << fileline << std::endl;
			return false;
		}
	}
	if (std::getline(file, fileline, '\n')
	|| std::getline(expect, expectline, '\n'))
	{
		logfile << "Number of lines doesn't match." << std::endl;
		return false;
	}
	return true;
}

bool test_file_write()
{
	Log					log;
	std::stringstream	ss;

	log.set_output(new std::ofstream(test_file, \
		std::ios_base::ate | std::ios_base::app), true);
	log << "Opened the file" << std::endl
		<< "In appending mode" << std::endl
		<< " for each write call" << std::endl;
	log << "Write another line" << std::endl;
	log.set_output(0, false);
	ss << "Opened the file" << std::endl
		<< "In appending mode" << std::endl
		<< " for each write call" << std::endl
		<< "Write another line" << std::endl;
	return check_content(test_file, ss);
}
