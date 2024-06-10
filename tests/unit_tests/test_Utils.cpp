#include <cstdlib>
#include <iostream>

#include "Utils.hpp"
#include "Log.hpp"

namespace {
	Log					log;
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

template<typename T>
static bool test_num_tostr(T input, std::string const &expect)
{
	std::string	result = utils::num_tostr(input);
	if (result != expect) {
		log << Log::ERROR << "Failed num_tostr" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return true;
}
template<typename T>
static bool test_str_tonum(std::string const &input, T expect)
{
	T result = utils::str_tonum<T>(input);
	if (result != expect){
		log << Log::ERROR << "Failed str_tonum" << std::endl
			<< "Input:		" << input << std::endl
			<< "Expected:	" << expect << std::endl
			<< "Result:		" << result << std::endl;
		return false;
	}
	return true;
}
static bool test_trim(std::string const &input, std::string const &trimchars,
		std::string const &expect);

int main()
{
	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);

	std::string const	loremipsum = "	  Lorem ipsum dolor sit amet, \
		onsectetur adipiscing elit, sed do eiusmod tempor incididunt ut \
		labore et dolore magna aliqua.\r	 ";
	std::string const	loremipsum_nospace = "	  Lorem ipsum dolor sit amet, \
		onsectetur adipiscing elit, sed do eiusmod tempor incididunt ut \
		labore et dolore magna aliqua.\r	";
	std::string const	loremipsum_notabspace = "Lorem ipsum dolor sit amet, \
		onsectetur adipiscing elit, sed do eiusmod tempor incididunt ut \
		labore et dolore magna aliqua.\r";
	std::string const	loremipsum_nowhitespace = "Lorem ipsum dolor sit amet, \
		onsectetur adipiscing elit, sed do eiusmod tempor incididunt ut \
		labore et dolore magna aliqua.";
	std::string const	punc_is_not_dead = "..!!!?!?!:;|^-^|;:?;:?;;,.,,!!";
	std::string const	longlines = "                  \r\n                   ";
	std::string const	space = " ";
	std::string const	tabspace = " \t";
	std::string const	whitespace = " \t\n\v\f\r";
	std::string const	punctuation = ",;:.?!";

	if (!test_trim(loremipsum, whitespace, loremipsum_nowhitespace)
	|| !test_trim(loremipsum, space, loremipsum_nospace)
	|| !test_trim(loremipsum, tabspace, loremipsum_notabspace)
	|| !test_trim(longlines, space, std::string("\r\n"))
	|| !test_trim(longlines, whitespace, std::string())
	|| !test_trim(std::string(), whitespace, std::string())
	|| !test_trim(punc_is_not_dead, punctuation, std::string("|^-^|")))
		return EXIT_FAILURE;
	if (!test_num_tostr(42L, "42")
	|| !test_num_tostr(0L, "0")
	|| !test_num_tostr(1000L, "1000")
	|| !test_num_tostr(-999L, "-999")
	|| !test_num_tostr(4242.42, "4242.42")
	|| !test_num_tostr(0.001, "0.001")
	|| !test_num_tostr(-0.00042, "-0.00042")
	|| !test_num_tostr(2147483649L, "2147483649")
	|| !test_num_tostr(-1L, "-1")
	|| !test_num_tostr("f00bar", "f00bar")
	|| !test_num_tostr("420123", "420123")) {
		return EXIT_FAILURE;
	}
	if (!test_str_tonum("42", 42L)
	|| !test_str_tonum("0", 0L)
	|| !test_str_tonum("1000", 1000L)
	|| !test_str_tonum("-999", -999L)
	|| !test_str_tonum("4242.42", 4242.42)
	|| !test_str_tonum("0.001", 0.001)
	|| !test_str_tonum("-0.00042", -0.00042)
	|| !test_str_tonum("2147483649", 2147483649L)
	|| !test_str_tonum("-1", -1L)
	|| !test_str_tonum("f00bar", 0)
	|| !test_str_tonum("f00bar1", 0)
	|| !test_str_tonum("420123", 420123L)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

bool test_trim(std::string const &input, std::string const &trimchars,
		std::string const &expect)
{
	std::string const result = utils::trim(input, trimchars);
	if (result == expect)
		return true;
	std::cerr << "Failed test_trim: " << std::endl
		<< "	Input:		" << input << std::endl
		<< "	Trim:		" << trimchars << std::endl
		<< "	Expected:	" << expect << std::endl
		<< "	Received:	" << result << std::endl;
	return false;
}
