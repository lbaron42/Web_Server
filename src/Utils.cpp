/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:59:51 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/26 22:27:47 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils.hpp"

std::string itoa(int num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

int atoi(const std::string &str) {
	std::istringstream ss(str);
	int num;
	ss >> num;
	return num;
}

std::string trim(std::string const &str, std::string const &trimchars)
{
	std::string::size_type start = str.find_first_not_of(trimchars);
	if (start == std::string::npos)
		return std::string();
	std::string::size_type end = str.find_last_not_of(trimchars);
	return str.substr(start, end - start + 1);
}

std::string c_trim(const std::string &str) {

	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return std::string();
	}

	size_t semicolon_pos = str.find(';');
	std::string trimmed;
	if (semicolon_pos != std::string::npos) {
		trimmed = str.substr(0, semicolon_pos + 1);
	} else {
		trimmed = str;
	}

	size_t last = trimmed.find_last_not_of(' ');
	return trimmed.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string &s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(trim(token));
	}
	return tokens;
}

ssize_t get_file_size(std::string const &filename)
{
	struct stat stat_buf;
	return (!stat(filename.c_str(), &stat_buf) ? stat_buf.st_size : -1);
}

std::stringstream::pos_type size_of_stream(const std::stringstream& ss)
{
	std::streambuf* buf = ss.rdbuf();
	std::stringstream::pos_type bckup = buf->pubseekoff(0, ss.cur, ss.out);
	std::stringstream::pos_type end = buf->pubseekoff(0, ss.end, ss.out);
	buf->pubseekpos(bckup, ss.out);
	return end;
}

static std::map<std::string, std::string> const mime_map = MimeType::init_map();

std::string const get_mime_type(std::string const &file)
{
	std::string::size_type	pos = file.find_last_of('.');

	if (pos == std::string::npos) {
		return std::string("text/plain");
	}
	std::string	ext = file.substr(pos + 1);
	std::map<std::string, std::string>::const_iterator it = mime_map.find(ext);
	if (it == mime_map.end()) {
		return std::string("text/plain");
	}
	return it->second;
}

bool isDigitString(const std::string& str) {
	if (str.empty()) {
		return false;
	}
	if (str.find_first_not_of("0123456789") != std::string::npos) {
		return false;
	}
	return true;
}

bool is_uint(std::string const &str)
{
	long	result;
	char	*end;

	if (str.empty())
		return false;
	errno = 0;
	result = std::strtol(str.c_str(), &end, 10);
	return (errno != ERANGE && *end == 0 && result >= 0);
}
