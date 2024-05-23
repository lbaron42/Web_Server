#include <cstdlib>
#include <iostream>
#include "Utils.hpp"

static bool test_trim(std::string const &input, std::string const &trimchars,
		std::string const &expect);

int main()
{
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
	return EXIT_SUCCESS;
}

bool test_trim(std::string const &input, std::string const &trimchars,
		std::string const &expect)
{
	std::string const result = trim(input, trimchars);
	if (result == expect)
		return true;
	std::cerr << "Failed test_trim: " << std::endl
		<< "	Input:		" << input << std::endl
		<< "	Trim:		" << trimchars << std::endl
		<< "	Expected:	" << expect << std::endl
		<< "	Received:	" << result << std::endl;
	return false;
}
