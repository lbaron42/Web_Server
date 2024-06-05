/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:59:51 by lbaron            #+#    #+#             */
/*   Updated: 2024/06/05 15:13:50 by mcutura          ###   ########.fr       */
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

bool try_file(std::string const &path)
{
	struct stat	sb;
	return (!stat(path.c_str(), &sb) && S_ISREG(sb.st_mode));
}

inline bool icompare_pred(unsigned char lhs, unsigned char rhs)
{
	return (std::tolower(lhs) == std::tolower(rhs));
}

bool icompare(std::string const &lhs, std::string const &rhs)
{
	if (lhs.length() == rhs.length())
		return std::equal(rhs.begin(), rhs.end(), lhs.begin(), icompare_pred);
	return false;
}

bool ends_with(std::string const &str, std::string const &end)
{
	if (str.length() < end.length())
		return false;
	return std::equal(end.rbegin(), end.rend(), str.rbegin());
}

std::string get_delimited(std::istream &in, std::string const &delimiter)
{
	std::string	result;
	std::string	chunk;
	size_t		d_size = delimiter.length();
	size_t		r(0);
	char		delim = *delimiter.rbegin();

	while (std::getline(in, chunk, delim)) {
		if (in.eof())
			return (result + chunk);
		result += chunk + delim;
		r = result.length();
		if (r >= d_size && result.substr(r - d_size, d_size) == delimiter)
			return result;
	}
	return result;
}

bool save_file(std::string name, std::vector<char> content, bool bin)
{
	std::ofstream	ofs;
	if (bin)
		ofs.open(name.c_str(), std::ios_base::trunc | std::ios_base::binary);
	else
		ofs.open(name.c_str(), std::ios_base::trunc);
	if (!ofs.is_open())
		return false;
	ofs.write(content.data(), content.size());
	ofs.close();
	return true;
}

std::string time_tostr(std::time_t &time)
{
	char	timestr[32];

	if (std::strftime(
		timestr,
		sizeof timestr,
		"%a, %d %b %Y %H:%M:%S %Z",
		std::gmtime(&time)))
		return std::string(timestr);
	return std::string();
}

bool str_tohex(std::string const &str, size_t *out_hex)
{
	std::stringstream	ss;
	if (str.empty())
		return false;
	ss << std::hex << str;
	if (ss.eof() && !ss.fail())
		ss >> *out_hex;
	return ss.eof() && !ss.fail();
}
