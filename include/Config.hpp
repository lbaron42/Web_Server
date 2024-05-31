/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/31 13:43:52 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

# include "Utils.hpp"
# include "Request.hpp"
# include "Server.hpp"

const int errorCodes[] = {
    100, // Continue
    101, // Switching Protocols
    200, // OK
    201, // Created
    202, // Accepted
    203, // Non-Authoritative Information
    204, // No Content
    205, // Reset Content
    206, // Partial Content
    300, // Multiple Choices
    301, // Moved Permanently
    302, // Found
    303, // See Other
    304, // Not Modified
    305, // Use Proxy
    307, // Temporary Redirect
    400, // Bad Request
    401, // Unauthorized
    402, // Payment Required
    403, // Forbidden
    404, // Not Found
    405, // Method Not Allowed
    406, // Not Acceptable
    407, // Proxy Authentication Required
    408, // Request Timeout
    409, // Conflict
    410, // Gone
    411, // Length Required
    412, // Precondition Failed
    413, // Request Entity Too Large
    414, // Request-URI Too Long
    415, // Unsupported Media Type
    416, // Requested Range Not Satisfiable
    417, // Expectation Failed
    500, // Internal Server Error
    501, // Not Implemented
    502, // Bad Gateway
    503, // Service Unavailable
    504, // Gateway Timeout
    505, // HTTP Version Not Supported
};

const int SERVER_TAB = 1;
const int LOCATION_TAB = 2;
const std::string IDENT = "\nserver {\n"
                          "\t[TAB][KEY][SP][VALUE][;] // variables\n\n"
                          "\tlocation /path {\t\t[TAB][TAB][KEY][SP][VALUE][;] // location variables\n"
                          "\t}\n"
                          "}";



class Config {
public:
	Config(Log &log);
	~Config();
	int configInit(const std::string& argv1);
	std::vector<Server> getServers() const;

private:
	Log &log;
	std::vector<Server> servers;
	bool verifyIp(std::string ip, int lineNum);
	bool verifyPort(std::string Port, int lineNum);
	bool validError(int error, int lineNum);
	bool validIndentation(std::string line, int tabNum, int lineNum);
	bool trimLine(const std::string& line, const std::string& message, int lineNum, std::string& trimmedLine);
	bool getAddress(std::string line, ServerData &current, int lineNum);
	bool getErrors(std::string line, ServerData &current, int lineNum);
};

std::ostream &operator<<(std::ostream &os, const ServerData &data);

#endif // CONFIG_HPP

