/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/06/15 14:52:16 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(Log &log) : log(log)
{}

Config::~Config()
{}

bool Config::verifyIp(std::string ip, int lineNum)
{
	// size_t temp;
	// std::vector<std::string> split_ip;
	// split_ip = utils::split(ip, '.');
	// for(std::vector<std::string>::const_iterator it = split_ip.begin(); it != split_ip.end(); ++it)
	// {
	// 	temp = atoi(it->c_str());
	// 	if(!utils::isDigitString(*it) || temp > 255)
	// 	{
	// 		log << log.ERROR << ".conf error: Invalid IP address on line: " << lineNum << std::endl;
	// 		return false;
	// 	}
	// }
	if (!utils::is_valid_ip4(utils::trim(ip))) {
		log << log.ERROR << ".conf error: Invalid IP address on line: "
			<< lineNum << std::endl
			<< "IP: [" << utils::trim(ip) << "]" << std::endl;
		return false;
	}
	return true;
}

bool Config::verifyPort(std::string port, int lineNum)
{
	size_t p = atoi(port.c_str());
	if(!utils::isDigitString(port) || p > 65535)
	{
		log << log.ERROR << ".conf error: Port number is not a \"digit\" or it is out of Range, line: " << lineNum << std::endl;
		return false;
	}
	return true;
}

bool Config::validError(int error, int lineNum)
{
	bool found = false;
	for (size_t i = 0; i < sizeof(errorCodes) / sizeof(errorCodes[0]); ++i)
	{
		if (error == errorCodes[i])
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		log << Log::ERROR << ".conf error: Error code doesn't exist: " << error <<  " line: " << lineNum << std::endl;
		return false;
	}
	return true;
}

bool Config::validIndentation(std::string line, int tabNum, int lineNum)
{
	int tab = tabNum;
	for (int i = 0; i < tab; ++i)
	{
		if(line[i] != '\t' || line[tabNum] == '\t')
		{
			log << log.ERROR << "Wrong indentation on line: " << lineNum << IDENT <<std::endl;
			return true;
		}
	}
	return false;
}

bool Config::trimLine(const std::string& line, const std::string& message, int lineNum, std::string& trimmedLine)
{
	size_t pos = line.find(message);
	if (pos == std::string::npos)
	{
		log << log.ERROR << ".conf error: Expected '" << message << "' in line: " << lineNum << std::endl;
		return false;
	}
	pos += message.length();
	while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) { pos++; }
	size_t end_pos = line.find(';', pos);
	if (end_pos == std::string::npos)
	{
		log << log.ERROR << ".conf error: Expected ';' in line: " << lineNum << " after " << message << std::endl;
		return false;
	}
	trimmedLine = line.substr(pos, end_pos - pos);
	return true;
}

bool Config::getAddress(std::string line, ServerData &current, int lineNum)
{
	ServerData::Address _address;
	std::string newLine;
	if (!trimLine(line, "listen", lineNum, newLine)) {
		return false;
	}
	size_t end_pos = newLine.find(':');
	if (end_pos == std::string::npos) {
		_address.ip = "0.0.0.0";
		_address.port = newLine;
		if (!verifyPort(_address.port, lineNum))
			return false;
		current.addresses.push_back(_address);
		return true;
	}
	_address.ip = newLine.substr(0, end_pos);
	if (!verifyIp(_address.ip, lineNum))
		return false;
	_address.port = newLine.substr(end_pos + 1);
	if (!verifyPort(_address.port, lineNum))
		return false;
	current.addresses.push_back(_address);
	return true;
}

bool Config::getErrors(std::string line, ServerData &current, int lineNum)
{
	std::string trimmedLine;
	if (!trimLine(line, "error_page", lineNum, trimmedLine)) {
		return false;
	}
	std::vector<std::string> splitError = utils::split(trimmedLine, ' ');
	if(splitError.size() != 2 || !utils::isDigitString(splitError[0]))
	{
		log << log.ERROR << "Wrong number of error_page elements on line: " << lineNum << " Example: 504 /50x.html" << std::endl;
		return false;
	}
	int temp = atoi(splitError[0].c_str());
	if (!validError(temp, lineNum))
	{
		return false;
	}
	current.error_pages.insert(std::pair<int, std::string>(temp, splitError[1]));
	return true;
}

int Config::configInit(const std::string &argv1)
{
	int lineNum = 0;
	std::ifstream config_file(argv1.c_str());
	if (!config_file.is_open()) {
		log << Log::ERROR << "Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	std::string line;
	while (std::getline(config_file, line)) {
		lineNum++;
		line = utils::c_trim(line);
		line = utils::trim(line, "\t");
		if (line.empty() || line[0] == '#') {
			continue;
		}
		if (line.find("server {") != std::string::npos)
		{
			ServerData sd;
			while (std::getline(config_file, line) && line.find('}') == std::string::npos)
			{
				lineNum++;
				line = utils::c_trim(line);
				if (line.empty() || line[0] == '#') {
					continue;
				}
				if(validIndentation(line, SERVER_TAB, lineNum))
					return EXIT_FAILURE;
				line = utils::trim(line, "\t");
				if (line.find("location") != std::string::npos)
				{
					ServerData::Location loc;
					size_t pos = line.find("location") + 9;
					while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) { pos++; }
					size_t end_pos = line.find('{', pos);
					loc.location_path = utils::trim(line.substr(pos, end_pos - pos));
					while (std::getline(config_file, line) && line.find('}') == std::string::npos)
					{
						lineNum++;
						line = utils::c_trim(line);
						if (line.empty() || line[0] == '#') {
							continue;
						}
						if(validIndentation(line, LOCATION_TAB, lineNum))
							return EXIT_FAILURE;
						line = utils::trim(line, "\t");
						std::string trimmed;
						if (line.find("alias") != std::string::npos)
						{
							if (!trimLine(line, "alias", lineNum, trimmed)) return EXIT_FAILURE;
							loc.alias = trimmed;
						}
						else if (line.find("autoindex") != std::string::npos)
						{
							loc.autoindex = false;
							if (!trimLine(line, "autoindex", lineNum, trimmed)) return EXIT_FAILURE;
							if (trimmed != "on" && trimmed != "off")
							{
								log << log.ERROR << ".conf error: autoindex is invalid on line: " << lineNum << std::endl;
								return EXIT_FAILURE;
							}
							else if (trimmed == "on")
								loc.autoindex = true;
						}
						else if (line.find("index") != std::string::npos)
						{
							if (!trimLine(line, "index", lineNum, trimmed)) return EXIT_FAILURE;
							loc.loc_index = utils::split(trimmed, ' ');
						}
						else if (line.find("allow_methods") != std::string::npos)
						{
							if (!trimLine(line, "allow_methods", lineNum, trimmed)) return EXIT_FAILURE;
							loc.allow_methods = Request::parse_methods(trimmed);
						}
						else if(line.find("return") != std::string::npos)
						{
							if (!trimLine(line, "return", lineNum, trimmed)) return EXIT_FAILURE;
							loc.redirection = trimmed;
						}
						else
						{
							log << log.ERROR << ".conf error: Wrong syntax on line: " << lineNum << std::endl;
							return EXIT_FAILURE;
						}
					}
					lineNum++;
					sd.locations.push_back(loc);
				}
				else if (line.find("listen") != std::string::npos)
				{
					if (!getAddress(line, sd, lineNum))
						return EXIT_FAILURE;
				}
				else if (line.find("server_name") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "server_name", lineNum, trimmed)) return EXIT_FAILURE;
					sd.hostnames = utils::split(trimmed, ' ');
				}
				else if (line.find("error_page") != std::string::npos)
				{
					if (!getErrors(line, sd, lineNum))
						return EXIT_FAILURE;
				}
				else if (line.find("autoindex") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "autoindex", lineNum, trimmed)) return EXIT_FAILURE;
					if (trimmed != "on" && trimmed != "off")
					{
						log << log.ERROR << ".conf error: autoindex is invalid on line: " << lineNum << std::endl;
						return EXIT_FAILURE;
					}
					else if (trimmed == "on")
						sd.autoindex = true;
				}
				else if (line.find("index") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "index", lineNum, trimmed)) return EXIT_FAILURE;
					sd.serv_index = utils::split(trimmed, ' ');
				}
				else if (line.find("root") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "root", lineNum, trimmed)) return EXIT_FAILURE;
					sd.root = trimmed;
				}
				else if (line.find("client_max_body_size") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "client_max_body_size", lineNum, trimmed)) return EXIT_FAILURE;
					if (!utils::is_uint(trimmed.c_str())) {
						log << Log::ERROR << "Invalid client_max_body_size, unsigned int expected" << std::endl;
						return EXIT_FAILURE;
					}
					sd.client_max_body_size = utils::str_tonum<size_t>(trimmed.c_str());
				}
				else if (line.find("allow_methods") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "allow_methods", lineNum, trimmed)) return EXIT_FAILURE;
					sd.allow_methods = Request::parse_methods(trimmed);
				}
				else if  (line.find("cgi_path") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "cgi_path", lineNum, trimmed)) return EXIT_FAILURE;
					sd.cgi_path = trimmed;
				}
				else if  (line.find("cgi_ext") != std::string::npos)
				{
					std::string trimmed;
					if (!trimLine(line, "cgi_ext", lineNum, trimmed)) return EXIT_FAILURE;
					sd.cgi_ext = utils::split(trimmed, ' ');
				}
				else
				{
					log << log.ERROR << ".conf error: Wrong syntax on line: " << lineNum << std::endl;
					return EXIT_FAILURE;
				}
			}
			if(sd.root.empty() || sd.addresses.empty())
			{
				log << log.ERROR << "Server Block need: \"root\" and valid \"0.0.0.0:validPortNumber" << std::endl;
				return EXIT_FAILURE;
			}
			lineNum++;
			servers.push_back(Server(sd, log));
		}
		else
		{
			log << log.ERROR << ".conf error: Wrong syntax on line: " << lineNum << std::endl;
			return EXIT_FAILURE;
		}
	}
	config_file.close();
	return EXIT_SUCCESS;
}

std::vector<Server> Config::getServers() const {
	return servers;
}

int s = 1;
std::ostream& operator<<(std::ostream& os, const ServerData &data)
{
	os << "\n                ServerData BLOCK: " << s << "\n" << std::endl;
	s++;
	//print ip and port addresses
	for (std::vector<ServerData::Address>::const_iterator addr_it = data.addresses.begin(); addr_it != data.addresses.end(); ++addr_it)
	{
		const ServerData::Address& address = *addr_it;
		os << "Server ip: " << address.ip << "\n";
		os << "Server port: " << address.port << "\n";
	}

	// Print hostnames
	os << "\nHostnames:\n\n";
	for (std::vector<std::string>::const_iterator host_it = data.hostnames.begin(); host_it != data.hostnames.end(); ++host_it)
	{
		os << "	" << *host_it << "\n";
	}

	// Print error pages
	os << "\nError pages:\n\n";
	for (std::map<int, std::string>::const_iterator err_it = data.error_pages.begin(); err_it != data.error_pages.end(); ++err_it)
	{
		os << "Error code: " << err_it->first << " Page: " << err_it->second << "\n";
	}

	// Print server index
	os << "Server index:\n";
	for (std::vector<std::string>::const_iterator idx_it = data.serv_index.begin(); idx_it != data.serv_index.end(); ++idx_it)
	{
		os << *idx_it << "\n";
	}

	// Print root
	os << "Root: " << data.root << "\n";

	// Print client max body size
	os << "Client max body size: " << data.client_max_body_size << "\n";

	// Print autoindex
	os << "Autoindex: " << (data.autoindex ? "on" : "off") << "\n";

	// Print allow methods
	os << "Allowed methods:\n";
	// for (std::vector<std::string>::const_iterator method_it = data.allow_methods.begin(); method_it != data.allow_methods.end(); ++method_it)
	// {
	// os << "		"<< *method_it << "\n";
	// }

	// Print locations
	os << "\nLocations:\n";
	int i = 1;
	for (std::vector<ServerData::Location>::const_iterator loc_it = data.locations.begin(); loc_it != data.locations.end(); ++loc_it)
	{
		os << "	Location n: " << i << std::endl;
		i++;
		const ServerData::Location& location = *loc_it;
		os << "		Location path: " << location.location_path << "\n";
		os << "		Alias: " << location.alias << "\n";

		os << "		Location index: ";
		for (std::vector<std::string>::const_iterator loc_idx_it = location.loc_index.begin(); loc_idx_it != location.loc_index.end(); ++loc_idx_it)
		{
			os << *loc_idx_it << " ";
		}

		os << "\n		Allowed methods:\n";
		// for (std::vector<std::string>::const_iterator loc_method_it = location.allow_methods.begin(); loc_method_it != location.allow_methods.end(); ++loc_method_it)
		// {
		// os << "				"<< *loc_method_it << "\n";
		// }

		os << "		Redirection: " << (location.autoindex ? "yes" : "no") << "\n";
	}

	return os;
}
