/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/22 19:58:32 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Utils.hpp"

void Config::getAddress(std::string line, ServerData *current)
{
	ServerData::Address address;
	size_t pos = line.find("listen");
	if (pos == std::string::npos) {std::cerr << "Invalid configuration line: " + line; exit(EXIT_FAILURE);}
	pos += 6;
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
            current->_address.push_back(address);
            return;
        }
        address.ip = line.substr(pos, end_pos - pos);
        pos = end_pos + 1;
    }
    size_t end_pos = line.find(';', pos);
    if (end_pos == std::string::npos) {std::cerr << "Expected ';' after port in line: " + line; exit(EXIT_FAILURE);}
    address.port = line.substr(pos, end_pos - pos);
    current->_address.push_back(address);
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
		if (line.find("server {") != std::string::npos)
		{
			ServerData Server;
			servers.push_back(Server);
			currentServer = &servers.back();
		}
		if (line.find("location") != std::string::npos)
		{
			ServerData::Location loc;
			currentServer->locations.push_back(loc);
			currentLoc = &currentServer->locations.back();
		}
		if (line.find("listen") != std::string::npos)
		{
			getAddress(line, currentServer);
		}
		if (line.find("root") != std::string::npos)
		{
			size_t pos = line.find("root") + 4;
			while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
			size_t end_pos = line.find(';', pos);
			if (end_pos == std::string::npos) {std::cerr << "Expected ';' after root in line: " + line; exit(EXIT_FAILURE);}
			currentServer->root = line.substr(pos, end_pos - pos);
		}
		if (line.find("client_max_body_size") != std::string::npos)
		{
			size_t pos = line.find("client_max_body_size") + 20;
			while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
			size_t end_pos = line.find(';', pos);
			if (end_pos == std::string::npos) {std::cerr << "Expected ';' after client_max_body_size in line: " + line; exit(EXIT_FAILURE);}
			currentServer->client_max_body_size = atoi(line.substr(pos, end_pos - pos));
		}
	}
	config_file.close();
	return EXIT_SUCCESS;
}

void Config::log() const
{
	for (std::vector<ServerData>::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		const ServerData &server = *it;
		std::cout << "\nRoot: " << server.root;
		std::cout << "\nclient_max_body_size: " << server.client_max_body_size;
		for(std::vector<ServerData::Address>::const_iterator addr_it = server._address.begin(); addr_it != server._address.end(); ++addr_it)
		{
			const ServerData::Address &address = *addr_it;
			std::cout << "\nServer ip: " << address.ip;
			std::cout << "\nServer port: " << address.port << "\n";
		}
	}
	// for (size_t i = 0; i < servers.size(); ++i) {
	//     const s_Server& server = servers[i];
	//     std::cout << "\n" << server.index_name << " index: " << server.index << "\n\n";
	//     for (std::map<std::string, std::string>::const_iterator it = server.servKeywords.begin(); it != server.servKeywords.end(); ++it) {
	//         std::cout << "  " << it->first << ": " << it->second << "\n";
	//     }
	//     for (std::map<std::string, s_Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it) {
	//         std::cout << "  location: " << loc_it->first << "\n";
	//         const s_Location& location = loc_it->second;
	//         for (std::map<std::string, std::string>::const_iterator dir_it = location.directives.begin(); dir_it != location.directives.end(); ++dir_it) {
	//             std::cout << "    " << dir_it->first << ": " << dir_it->second << "\n";
	//         }
	//     }
	// }
}

// const std::vector<s_Server>& Config::getServers() const {
//     return servers;
// }
