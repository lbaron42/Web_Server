/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/26 16:01:38 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(Log &log) : log(log)
{}

Config::~Config()
{}

void Config::verifyIp(std::string ip, int lineNum)
{
	int temp;
	std::vector<std::string> split_ip;
	split_ip = split(ip, '.');
	for(std::vector<std::string>::const_iterator it = split_ip.begin(); it != split_ip.end(); ++it)
	{
		temp = atoi(*it);
		if(!isDigitString(*it) || (temp < 0 || temp > 255))
		{
			log << log.ERROR << ".conf error: Invalid IP address on line: " << lineNum << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

void Config::verifyPort(std::string port, int lineNum)
{
	size_t p = atoi(port);
	if(!isDigitString(port) || p > 65535)
	{
		log << log.ERROR << ".conf error: Port number is not a \"digit\" or it is out of Range, line: " << lineNum << std::endl;
		exit(EXIT_FAILURE);
	}

}

void Config::validError(int error, int lineNum)
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
		exit(EXIT_FAILURE);
	}
}

std::string Config::trimLine(std::string line, std::string message, int lineNum)
{
	size_t pos = line.find(message) + message.length();
	while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
	size_t end_pos = line.find(';', pos);
	if (end_pos == std::string::npos)
	{
		trim(line);
		log << log.ERROR << ".conf error: Expected ';' in line: " << lineNum  << " " << message << std::endl;
		exit(EXIT_FAILURE);
	}
	return line.substr(pos, end_pos - pos);
}

void Config::getAddress(std::string line, ServerData &current, int lineNum)
{
	ServerData::Address _address;
	std::string newLine = trimLine(line, "listen", lineNum);
	if(isdigit(newLine[0]) != 1 /* && Address[0] != '['*/)
	{
		log << log.ERROR << ".conf error: Invalid IP address on line: " << lineNum << std::endl;
		exit(EXIT_FAILURE);
	}
	size_t pos = 0;
		size_t end_pos = newLine.find(':', pos);
		if (end_pos == std::string::npos)
		{
			_address.ip = "0.0.0.0";
			_address.port = newLine;
			verifyPort(_address.port, lineNum);
			current.addresses.push_back(_address);
			return;
		}
		_address.ip = newLine.substr(pos, end_pos - pos);
		verifyIp(_address.ip, lineNum);
		pos = end_pos + 1;
		end_pos = std::string::npos;
		_address.port = newLine.substr(pos, end_pos - pos);
		verifyPort(_address.port, lineNum);
		current.addresses.push_back(_address);
}

void Config::getErrors(std::string line, ServerData &current, int lineNum)
{
	std::vector<std::string> splitError = split(trimLine(line, "error_page", lineNum), ' ');
	for(std::vector<std::string>::const_iterator split_it = splitError.begin(); split_it != splitError.end(); ++split_it)
		{
			const std::string &split = *split_it;
			if(isDigitString(split))
			{
				int temp = atoi(split);
				validError(temp, lineNum);
				current.error_pages.push_back(std::make_pair(temp, split));
			}
			else
				current.error_pages.push_back(std::make_pair(-1, split));
		}
}

int Config::configInit(const std::string &argv1)
{
	int lineNum = 0;
	std::ifstream config_file(argv1.c_str());
	if (!config_file.is_open()) {
		log << log.ERROR << "Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	std::string line;
	while (std::getline(config_file, line)) {
		lineNum++;
		line = c_trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		if (line.find("server {") != std::string::npos)
		{
			ServerData sd;
			while (std::getline(config_file, line) && line.find('}') == std::string::npos)
			{
				lineNum++;
				line = c_trim(line);
				if (line.find("location") != std::string::npos)
				{
					ServerData::Location loc;
					size_t pos = line.find("location") + 9;
					while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
					size_t end_pos = line.find('{', pos);
					loc.location_path = trim(line.substr(pos, end_pos - pos));
					while (std::getline(config_file, line) && line.find('}') == std::string::npos)
					{
						lineNum++;
						line = c_trim(line);
						if (line.find("alias") != std::string::npos)
						{
							loc.alias = trimLine(line, "alias", lineNum);
						}
						if (line.find("autoindex") != std::string::npos)
						{
							loc.is_redirection = false;
							std::string trimmed = trimLine(line, "autoindex", lineNum);
							if (trimmed != "on" && trimmed != "off")
							{
        		    			log << log.ERROR << ".conf error: autoindex is invalid on line: " << lineNum << std::endl;
								exit(EXIT_FAILURE);
							}
							else if (trimmed == "on")
        		    			loc.is_redirection = true;
						}
						if (line.find("index") != std::string::npos)
						{
							loc.loc_index = split(trimLine(line, "index", lineNum), ' ');
						}
						if (line.find("allow_methods") != std::string::npos)
		        		{
							loc.allow_methods = Request::parse_methods(trimLine(line, "allow_methods", lineNum));
						}
					}
					lineNum++;
					sd.locations.push_back(loc);
				}
				if (line.find("listen") != std::string::npos)
				{
					getAddress(line, sd, lineNum);
				}
				if (line.find("server_name") != std::string::npos)
				{
					sd.hostnames = split(trimLine(line, "server_name", lineNum), ' ');
				}
				if (line.find("error_page") != std::string::npos)
				{
					getErrors(line, sd, lineNum);
				}
				if (line.find("autoindex") != std::string::npos)
				{
					std::string trimmed = trimLine(line, "autoindex", lineNum);
					if (trimmed != "on" && trimmed != "off")
					{
        		    	log << log.ERROR << ".conf error: autoindex is invalid on line: " << lineNum << std::endl;
						exit(EXIT_FAILURE);
					}
					else if (trimmed == "on")
        		    	sd.autoindex = true;
				}
				if (line.find("index") != std::string::npos)
				{
					sd.serv_index = split(trimLine(line, "index", lineNum), ' ');
				}
				if (line.find("root") != std::string::npos)
				{
					sd.root = trimLine(line, "root", lineNum);
				}
				if (line.find("client_max_body_size") != std::string::npos)
				{
					sd.client_max_body_size = atoi(trimLine(line, "client_max_body_size", lineNum));
				}
				if (line.find("allow_methods") != std::string::npos)
				{
					// sd.allow_methods = split(trimLine(line, "allowed_method", lineNum), ' ');
					sd.allow_methods = Request::parse_methods(trimLine(line, "allow_methods", lineNum));
				}
			}
			lineNum++;
			// std::cout << sd << std::endl;
			servers.push_back(Server(sd, log));
		}
		else
		{
			log << log.ERROR << ".conf error: Wrong syntax on line: " << lineNum << std::endl;
			exit(EXIT_FAILURE);
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
    for (std::vector<std::pair<int, std::string> >::const_iterator err_it = data.error_pages.begin(); err_it != data.error_pages.end(); ++err_it)
    {
        os << "	Error code: " << err_it->first << " Page: " << err_it->second << "\n";
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
    //     os << "		"<< *method_it << "\n";
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
        //     os << "				"<< *loc_method_it << "\n";
        // }

        os << "		Redirection: " << (location.is_redirection ? "yes" : "no") << "\n";
    }

    return os;
}
