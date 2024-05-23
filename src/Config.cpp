/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/23 17:06:21 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Utils.hpp"

void Config::validError(int error)
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
		std::cerr << "ERROR: Code doesn't exist: " << error << std::endl;
		exit(EXIT_FAILURE);
	}
}

std::string Config::trimLine(std::string line, std::string message)
{
	size_t pos = line.find(message) + message.length();
	while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
	size_t end_pos = line.find(';', pos);
	if (end_pos == std::string::npos)
	{
		std::cerr << "Expected ';' " << message << " in line: " + line;
		exit(EXIT_FAILURE);
	}
	return line.substr(pos, end_pos - pos);
}

void Config::getAddress(std::string line, ServerData *current)
{
	ServerData::Address address;
	size_t pos = line.find("listen") + 6;
	while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
	if (line[pos] == '[') {
		size_t end_pos = line.find(']', pos);
		if (end_pos == std::string::npos) {std::cerr << "Invalid IPv6 address in line: " + line; exit(EXIT_FAILURE);}
		address.ip = line.substr(pos + 1, end_pos - pos - 1);
		pos = end_pos + 1;
		if (line[pos] != ':') {std::cerr << "Expected ':' after IPv6 address in line: " + line; exit(EXIT_FAILURE);}
		pos++;
	} else {
		size_t end_pos = line.find(':', pos);
		if (end_pos == std::string::npos) {
			address.ip = "";
			end_pos = line.find(';', pos);
			if (end_pos == std::string::npos) {std::cerr << "Invalid configuration line: " + line; exit(EXIT_FAILURE);}
			address.port = line.substr(pos, end_pos - pos);
			current->addresses.push_back(address);
			return;
		}
		address.ip = line.substr(pos, end_pos - pos);
		pos = end_pos + 1;
	}
	size_t end_pos = line.find(';', pos);
	if (end_pos == std::string::npos) {std::cerr << "Expected ';' after port in line: " + line; exit(EXIT_FAILURE);}
	address.port = line.substr(pos, end_pos - pos);
	current->addresses.push_back(address);
}

void Config::getErrors(std::string line, ServerData *current)
{
	std::vector<std::string> splitError = split(trimLine(line, "error_page"), ' ');
	for(std::vector<std::string>::const_iterator split_it = splitError.begin(); split_it != splitError.end(); ++split_it)
		{
			const std::string &split = *split_it;
			if(isDigitString(split))
			{
				int temp = atoi(split);
				validError(temp);
				current->error_pages.push_back(std::make_pair(temp, split));
			}
			else
				current->error_pages.push_back(std::make_pair(-1, split));
		}	
}

int Config::configInit(const std::string &argv1)
{
	std::ifstream config_file(argv1.c_str());

	if (!config_file.is_open())
	{
		std::cerr << "ERROR: Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	std::string line;
	ServerData *currentServer = NULL;
	ServerData::Location *currentLoc;
	while (std::getline(config_file, line))
	{
		line = trim(line);
		if (line.find("#") != std::string::npos || line.empty())
	        continue;
		if (line.find("server {") != std::string::npos)
		{
			ServerData Server;
			servers.push_back(Server);
			currentServer = &servers.back();
			currentServer->client_max_body_size = 1;
			currentServer->autoindex = false;
		}
		if (line.find("location") != std::string::npos)
		{
			ServerData::Location loc;
			currentServer->locations.push_back(loc);
			currentLoc = &currentServer->locations.back();
			size_t pos = line.find("location") + 9;
			while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
			size_t end_pos = line.find('{', pos);
			currentLoc->location_path = trim(line.substr(pos, end_pos - pos));
			while(std::getline(config_file, line) && line.find('}') == std::string::npos)
			{
				if (line.find("alias") != std::string::npos)
				{
					currentLoc->alias = trimLine(line, "alias");
				}
				if (line.find("index") != std::string::npos)
				{
					currentLoc->loc_index = split(trimLine(line, "alias"), ' ');
				}
				if (line.find("allow_methods") != std::string::npos)
		        {
			        currentLoc->allow_methods = split(trimLine(line, "allow_methods"), ' ');
		        }
			}
		}
		if (line.find("listen") != std::string::npos)
		{
			getAddress(line, currentServer);
		}
        if (line.find("server_name") != std::string::npos)
		{
			currentServer->hostnames = split(trimLine(line, "server_name"), ' ');
		}
        if (line.find("error_page") != std::string::npos)
        {
            getErrors(line, currentServer); 
        }
		if (line.find("index") != std::string::npos)
		{
			currentServer->serv_index = split(trimLine(line, "index"), ' ');
		}
		if (line.find("root") != std::string::npos)
		{
			currentServer->root = trimLine(line, "root");
		}
		if (line.find("client_max_body_size") != std::string::npos)
		{
			currentServer->client_max_body_size = atoi(trimLine(line, "client_max_body_size"));
		}
		if (line.find("autoindex") != std::string::npos)
		{
			std::string trimmed = trimLine(line, "autoindex");
			if (trimmed != "on" && trimmed != "off")
            	std::cerr << "autoindex is invalid on line: " << line << std::endl;
			else if (trimmed == "on")
            	currentServer->autoindex = true;	
		}
		if (line.find("allow_methods") != std::string::npos)
		{
			currentServer->allow_methods = split(trimLine(line, "allow_methods"), ' ');
		}
	}
	config_file.close();
	return EXIT_SUCCESS;
}

const std::vector<ServerData>& Config::getServers() const {
    return servers;
}

std::ostream& operator<<(std::ostream& os, const Config& config)
{
    for (std::vector<ServerData>::const_iterator it = config.getServers().begin(); it != config.getServers().end(); ++it) {
        const ServerData& server = *it;
        for (std::vector<ServerData::Address>::const_iterator addr_it = server.addresses.begin(); addr_it != server.addresses.end(); ++addr_it) {
            const ServerData::Address& address = *addr_it;
            os << "Server ip: " << address.ip << "\n";
            os << "Server port: " << address.port << "\n";
        }
        os << "Root: " << server.root << "\n";
        os << "client_max_body_size: " << server.client_max_body_size << "\n";
        os << "Hosts:\n";
        for (std::vector<std::string>::const_iterator host_it = server.hostnames.begin(); host_it != server.hostnames.end(); ++host_it) {
            const std::string& host = *host_it;
            os << "\t" << host << "\n";
        }
        for (std::vector<std::pair<int, std::string> >::const_iterator err_it = server.error_pages.begin(); err_it != server.error_pages.end(); ++err_it) {
            const std::pair<int, std::string>& err = *err_it;
            os << "error num: " << err.first << " error string: " << err.second << "\n";
        }
        os << "autoindex: " << server.autoindex << "\n";
        os << "Allow methods:\n";
        for (std::vector<std::string>::const_iterator method_it = server.allow_methods.begin(); method_it != server.allow_methods.end(); ++method_it) {
            const std::string& method = *method_it;
            os << "\t" << method << "\n";
        }
        os << "Locations:\n";
        for (std::vector<ServerData::Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it) {
            const ServerData::Location& location = *loc_it;
            os << "\tLocation Path: " << location.location_path << "\n";
            os << "\tAlias: " << location.alias << "\n";
            os << "\tIndex: ";
            for (std::vector<std::string>::const_iterator index_it = location.loc_index.begin(); index_it != location.loc_index.end(); ++index_it) {
                const std::string& index = *index_it;
                os << index << " ";
            }
            os << "\n\tAllow Methods: ";
            for (std::vector<std::string>::const_iterator method_it = location.allow_methods.begin(); method_it != location.allow_methods.end(); ++method_it) {
                const std::string& method = *method_it;
                os << method << " ";
            }
            os << "\n";
        }
        os << "\n";
    }
    return os;
}