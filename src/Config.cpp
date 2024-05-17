/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/17 12:21:05 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Utils.hpp"

Config::Config() {
    keyWords.push_back("listen");
    keyWords.push_back("server_name");
    keyWords.push_back("port_to_listen");
    keyWords.push_back("error_page");
    keyWords.push_back("client_max_body_size");
    keyWords.push_back("index");
}

int Config::configInit(const std::string& argv1) {
    std::ifstream config_file(argv1.c_str());
    
    if (!config_file.is_open()) {
        std::cerr << "ERROR: Couldn't open config file" << std::endl;
        return EXIT_FAILURE;
    }
    
    int set = 0;
    std::string line;
    s_Server* currentServer = NULL;
    
    while (std::getline(config_file, line)) {
        line = trim(line);
        
        if (line.find("server {") != std::string::npos) {
            set++;
            std::string setname = "Server" + itoa(set);
            s_Server obj(setname, set);
            servers.push_back(obj);
            currentServer = &servers.back();
        } else if (line.find("location") != std::string::npos && currentServer != NULL) {
            size_t pos = line.find("location");
            pos += 8;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
                pos++;
            }
            size_t end_pos = line.find('{', pos);
            std::string location_path = line.substr(pos, end_pos - pos);
            location_path = trim(location_path);
            
            s_Location location;
            while (std::getline(config_file, line)) {
                line = trim(line);
                if (line.find('}') != std::string::npos) {
                    break;
                }
                size_t directive_pos = line.find(' ');
                std::string directive = line.substr(0, directive_pos);
                std::string value = line.substr(directive_pos + 1, line.find(';') - directive_pos - 1);
                location.directives[directive] = trim(value);
            }
            currentServer->locations[location_path] = location;
        } else if (currentServer != NULL) {
            for (size_t i = 0; i < keyWords.size(); ++i) {
                size_t pos = line.find(keyWords[i]);
                if (pos != std::string::npos) {
                    pos += keyWords[i].length();
                    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
                        pos++;
                    }
                    size_t end_pos = line.find(';', pos);
                    if (end_pos != std::string::npos) {
                        std::string value = line.substr(pos, end_pos - pos);
                        currentServer->servKeywords[keyWords[i]] = value;
                    }
                }
            }
        }
    }
    config_file.close();
    return EXIT_SUCCESS;
}

void Config::log() const {
    for (size_t i = 0; i < servers.size(); ++i) {
        const s_Server& server = servers[i];
        std::cout << "\n" << server.index_name << " index: " << server.index << "\n\n";
        for (std::map<std::string, std::string>::const_iterator it = server.servKeywords.begin(); it != server.servKeywords.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << "\n";
        }
        for (std::map<std::string, s_Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it) {
            std::cout << "  location: " << loc_it->first << "\n";
            const s_Location& location = loc_it->second;
            for (std::map<std::string, std::string>::const_iterator dir_it = location.directives.begin(); dir_it != location.directives.end(); ++dir_it) {
                std::cout << "    " << dir_it->first << ": " << dir_it->second << "\n";
            }
        }
    }  
}

const std::vector<s_Server>& Config::getServers() const {
    return servers;
}