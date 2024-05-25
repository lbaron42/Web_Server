/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/25 06:41:04 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(Log &log) : log(log)
{}

Config::~Config()
{}

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
		log << Log::ERROR << "Code doesn't exist: " << error << std::endl;
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
		log << log.ERROR << "Expected ';' " << message << " in line: " + line << " " << lineNum << std::endl;
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
		log << log.ERROR << "ip address on line: " << lineNum <<  "is invalid" << std::endl;
		exit(EXIT_FAILURE);
	}
	size_t pos = 0;
		size_t end_pos = newLine.find(':', pos);
		if (end_pos == std::string::npos)
		{
			_address.ip = "0.0.0.0";
			_address.port = newLine;
			current.addresses.push_back(_address);
			return;
		}
		_address.ip = newLine.substr(pos, end_pos - pos);
		pos = end_pos + 1;
		end_pos = std::string::npos;
		_address.port = newLine.substr(pos, end_pos - pos);
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
				validError(temp);
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
					sd.locations.push_back(loc);
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
						if (line.find("index") != std::string::npos)
						{
							loc.loc_index = split(trimLine(line, "alias", lineNum), ' ');
						}
						if (line.find("allow_methods") != std::string::npos)
		        		{
							loc.allow_methods = split(trimLine(line, "allow_methods", lineNum), ' ');
						}
						if (line.find("autoindex") != std::string::npos)
						{
							loc.is_redirection = false;
							std::string trimmed = trimLine(line, "autoindex", lineNum);
							if (trimmed != "on" && trimmed != "off")
					 			log << log.ERROR << "autoindex is invalid on line: " << lineNum << std::endl;
							else if (trimmed == "on")
        		    			loc.is_redirection = true;
						}
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
					if (line.find("autoindex") != std::string::npos)
					{
						std::string trimmed = trimLine(line, "autoindex", lineNum);
						if (trimmed != "on" && trimmed != "off")
        			    	log << log.ERROR << "autoindex is invalid on line: " << lineNum << std::endl;
						else if (trimmed == "on")
        			    	sd.autoindex = true;
					}
					if (line.find("allow_methods") != std::string::npos)
					{
						sd.allow_methods = split(trimLine(line, "allow_methods", lineNum), ' ');
					}
				}
			}
			servers.push_back(Server(sd, log));
		}
		else
		{
			log << log.ERROR << "Wrong syntax on line: " << lineNum << " of the .conf file" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	config_file.close();
	return EXIT_SUCCESS;
}

const std::vector<Server>& Config::getServers() const {
    return servers;
}

// std::ostream& operator<<(std::ostream& os, const Config& config)
// {
//    for (std::vector<Server>::const_iterator it = config.getServers().begin(); it != config.getServers().end(); ++it)
//     {
//         const Server& server = *it;
//         for (std::vector<ServerData::Address>::const_iterator addr_it = .addresses.begin(); addr_it != server.addresses.end(); ++addr_it)
//         {
//             const ServerData::Address& address = *addr_it;
//             os << "Server ip: " << address.ip << "\n";
//             os << "Server port: " << address.port << "\n";
//         }
//     }
//     return os;
// }







// int Config::configInit(const std::string &argv1)
// {
// 	std::ifstream config_file(argv1.c_str());

// 	if (!config_file.is_open())
// 	{
// 		std::cerr << "ERROR: Couldn't open config file" << std::endl;
// 		return EXIT_FAILURE;
// 	}
// 	std::string line;
// 	ServerData *currentServer;
// 	ServerData::Location *currentLoc;
// 	while (std::getline(config_file, line))
// 	{
// 		line = trim(line);
// 		if (line.find("#") != std::string::npos || line.empty())
// 	        continue;
// 		if (line.find("server {") != std::string::npos)
// 		{
// 			ServerData Server;
// 			servers.push_back(Server);
// 			currentServer = &servers.back();
// 			sd.client_max_body_size = 1024;
// 			sd.autoindex = false;
// 		}
// 		if (line.find("location") != std::string::npos)
// 		{
// 			ServerData::Location loc;
// 			sd.locations.push_back(loc);
// 			currentLoc = &sd.locations.back();
// 			size_t pos = line.find("location") + 9;
// 			while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {pos++;}
// 			size_t end_pos = line.find('{', pos);
// 			sd.locations.location_path = trim(line.substr(pos, end_pos - pos));
// 			while(std::getline(config_file, line) && line.find('}') == std::string::npos)
// 			{
// 				if (line.find("alias") != std::string::npos)
// 				{
// 					sd.locations.alias = trimLine(line, "alias");
// 				}
// 				if (line.find("index") != std::string::npos)
// 				{
// 					sd.locations.loc_index = split(trimLine(line, "alias"), ' ');
// 				}
// 				if (line.find("allow_methods") != std::string::npos)
// 		        {
// 			        sd.locations.allow_methods = split(trimLine(line, "allow_methods"), ' ');
// 		        }
// 			}
// 		}
// 		if (line.find("listen") != std::string::npos)
// 		{
// 			getAddress(line, *currentServer);
// 		}
//         if (line.find("server_name") != std::string::npos)
// 		{
// 			sd.hostnames = split(trimLine(line, "server_name"), ' ');
// 		}
//         if (line.find("error_page") != std::string::npos)
//         {
//             getErrors(line, *currentServer);
//         }
// 		if (line.find("index") != std::string::npos)
// 		{
// 			sd.serv_index = split(trimLine(line, "index"), ' ');
// 		}
// 		if (line.find("root") != std::string::npos)
// 		{
// 			sd.root = trimLine(line, "root");
// 		}
// 		if (line.find("client_max_body_size") != std::string::npos)
// 		{
// 			sd.client_max_body_size = atoi(trimLine(line, "client_max_body_size"));
// 		}
// 		if (line.find("autoindex") != std::string::npos)
// 		{
// 			std::string trimmed = trimLine(line, "autoindex");
// 			if (trimmed != "on" && trimmed != "off")
//             	std::cerr << "autoindex is invalid on line: " << line << std::endl;
// 			else if (trimmed == "on")
//             	sd.autoindex = true;
// 		}
// 		if (line.find("allow_methods") != std::string::npos)
// 		{
// 			sd.allow_methods = split(trimLine(line, "allow_methods"), ' ');
// 		}
// 	}
// 	config_file.close();
// 	return EXIT_SUCCESS;
// }


// std::ostream& operator<<(std::ostream& os, const Config& config)
// {
//     for (std::vector<ServerData>::const_iterator it = config.getServers().begin(); it != config.getServers().end(); ++it) {
//         const ServerData& server = *it;
//         for (std::vector<ServerData::Address>::const_iterator addr_it = server.addresses.begin(); addr_it != server.addresses.end(); ++addr_it) {
//             const ServerData::Address& address = *addr_it;
//             os << "Server ip: " << address.ip << "\n";
//             os << "Server port: " << address.port << "\n";
//         }
//         os << "Root: " << server.root << "\n";
//         os << "client_max_body_size: " << server.client_max_body_size << "\n";
//         os << "Hosts:\n";
//         for (std::vector<std::string>::const_iterator host_it = server.hostnames.begin(); host_it != server.hostnames.end(); ++host_it) {
//             const std::string& host = *host_it;
//             os << "\t" << host << "\n";
//         }
//         for (std::vector<std::pair<int, std::string> >::const_iterator err_it = server.error_pages.begin(); err_it != server.error_pages.end(); ++err_it) {
//             const std::pair<int, std::string>& err = *err_it;
//             os << "error num: " << err.first << " error string: " << err.second << "\n";
//         }
//         os << "autoindex: " << server.autoindex << "\n";
//         os << "Allow methods:\n";
//         for (std::vector<std::string>::const_iterator method_it = server.allow_methods.begin(); method_it != server.allow_methods.end(); ++method_it) {
//             const std::string& method = *method_it;
//             os << "\t" << method << "\n";
//         }
//         os << "Locations:\n";
//         for (std::vector<ServerData::Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it) {
//             const ServerData::Location& location = *loc_it;
//             os << "\tLocation Path: " << location.location_path << "\n";
//             os << "\tAlias: " << location.alias << "\n";
//             os << "\tIndex: ";
//             for (std::vector<std::string>::const_iterator index_it = location.loc_index.begin(); index_it != location.loc_index.end(); ++index_it) {
//                 const std::string& index = *index_it;
//                 os << index << " ";
//             }
//             os << "\n\tAllow Methods: ";
//             for (std::vector<std::string>::const_iterator method_it = location.allow_methods.begin(); method_it != location.allow_methods.end(); ++method_it) {
//                 const std::string& method = *method_it;
//                 os << method << " ";
//             }
//             os << "\n";
//         }
//         os << "\n";
//     }
//     return os;
// }
