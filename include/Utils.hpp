/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:52:42 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/26 14:28:45 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <map>
# include <string>
# include <sstream>
# include <vector>

# include <sys/stat.h>
# include <sys/types.h>

template<typename T>
std::string num_tostr(T num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

template<typename T>
T str_tonum(std::string const &str)
{
	T					num = T();
	std::istringstream	oss(str);
	oss >> num;
	return num;
}

std::string itoa(int num);
int atoi(const std::string &str);
std::string trim(std::string const &str, std::string const &trimchars = " \t");
std::string c_trim(const std::string &str);
std::vector<std::string> split(const std::string &s, char delimiter);
std::string const get_mime_type(std::string const &file);
ssize_t get_file_size(std::string const &filename);
std::stringstream::pos_type size_of_stream(const std::stringstream& ss);
bool isDigitString(const std::string& str);

struct MimeType
{
	static std::map<std::string, std::string> init_map()
	{
		std::map<std::string, std::string> m;
		m[""] = "text/plain";
		m["aac"] = "audio/aac";
		m["avi"] = "video/x-msvideo";
		m["bin"] = "application/octet-stream";
		m["bmp"] = "image/bmp";
		m["css"] = "text/css";
		m["csv"] = "text/csv";
		m["doc"] = "application/msword";
		m["docx"] = "application/msword";
		m["epub"] = "application/epub+zip";
		m["flac"] = "audio/flac";
		m["gif"] = "image/gif";
		m["gz"] = "application/gzip";
		m["htm"] = "text/html";
		m["html"] = "text/html";
		m["ico"] = "image/ico";
		m["ics"] = "text/calendar";
		m["jar"] = "application/java-archive";
		m["jpeg"] = "image/jpeg";
		m["jpg"] = "image/jpeg";
		m["js"] = "text/javascript";
		m["json"] = "application/json";
		m["mjs"] = "text/javascript";
		m["md"] = "text/markdown";
		m["mp3"] = "audio/mpeg";
		m["mp4"] = "video/mp4";
		m["mpeg"] = "video/mpeg";
		m["oga"] = "audio/ogg";
		m["otf"] = "font/otf";
		m["pdf"] = "application/pdf";
		m["png"] = "image/png";
		m["php"] = "application/x-httpd-php";
		m["sh"] = "application/x-sh";
		m["sql"] = "application/sql";
		m["svg"] = "image/svg+xml";
		m["tiff"] = "image/tiff";
		m["ttf"] = "font/ttf";
		m["txt"] = "text/plain";
		m["wasm"] = "application/wasm";
		m["waw"] = "audio/waw";
		m["webp"] = "image/webp";
		m["xhtml"] = "application/xhtml+xml";
		m["xml"] = "application/xml";
		m["yaml"] = "application/yaml";
		m["zip"] = "application/zip";
		m["7z"] = "application/x-7z-compressed";
		return m;
	}
	static std::map<std::string, std::string> const mime_map;
};

#endif // UTILS_HPP
