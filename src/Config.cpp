/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/16 17:23:46 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
# include "Utils.hpp"


int Config::configInit(const std::string& argv1) {
    std::ifstream config_file(argv1.c_str());
    if (!config_file.is_open()) {
        std::cerr << "ERROR: Couldn't open config file" << std::endl;
        return EXIT_FAILURE;
    }
    std::string line;
    int set = 0;
    while (std::getline(config_file, line))
    {
        if (line.find("server {") != std::string::npos)
        {
            set++;
            std::string setname = "Server" + itoa(set);
            s_Server obj(setname, set);
            servers.push_back(obj);
        }
    }
    config_file.close();
    return EXIT_SUCCESS;
}

void Config::log() const {
    for (std::vector<s_Server>::const_iterator it = servers.begin(); it != servers.end(); ++it)
    {const s_Server& server = *it;
       std::cout << "Server Name: " << server.server_name << ", index: " << server.index << std::endl;
    }
}


//     std::string Config::trim(const std::string& str) const {
//     size_t first = str.find_first_not_of(' ');
//     if (std::string::npos == first) {
//         return "";
//     }
//     size_t last = str.find_last_not_of(' ');
//     return str.substr(first, (last - first + 1));
// }

// int Config::configInit(const std::string& filename) {
//     std::ifstream config_file(filename.c_str());
//     if (!config_file.is_open()) {
//         std::cerr << "ERROR: Couldn't open config file" << std::endl;
//         return EXIT_FAILURE;
//     }

//     std::string line;
//     while (std::getline(config_file, line)) {
//         line = trim(line);
//         if (line.empty()) continue;
//         if (line.find("server {") != std::string::npos) {
//             parseServer(config_file);
//         }
//     }
//     config_file.close();
//     return EXIT_SUCCESS;
// }

// void Config::parseServer(std::ifstream& config_file) {
//     s_Server server;
//     std::string line;

//     while (std::getline(config_file, line)) {
//         line = trim(line);
//         std::cout << "Parsing line: " << line << std::endl;  // Debugging

//         if (line.empty()) continue;
//         if (line == "}") {
//             servers.push_back(server); // End of server block, save the server
//             break;
//         }

//         if (line.find("listen") == 0) {
//             server.port_to_listen = trim(line.substr(7));
//         } else if (line.find("server_name") == 0) {
//             server.server_name = trim(line.substr(12));
//         } else if (line.find("root") == 0) {
//             server.root = trim(line.substr(5));
//         } else if (line.find("error_page") == 0) {
//             std::istringstream ss(line.substr(11));
//             int code;
//             std::string path;
//             while (ss >> code >> path) {
//                 server.error_pages[code] = path;
//             }
//         } else if (line.find("client_max_body_size") == 0) {
//             server.client_max_body_size = trim(line.substr(21));
//         } else if (line.find("location") == 0) {
//             std::string location_path = trim(line.substr(9, line.find("{") - 9));
//             parseLocation(config_file, server, location_path);
//         }
//     }
// }

// void Config::parseLocation(std::ifstream& config_file, s_Server& server, const std::string& location_path) {
//     s_Location location;
//     std::string line;

//     while (std::getline(config_file, line)) {
//         line = trim(line);
//         std::cout << "  Parsing location line: " << line << std::endl;  // Debugging

//         if (line.empty()) continue;
//         if (line == "}") {
//             break; // End of location block
//         }

//         std::istringstream ss(line);
//         std::string directive, value;
//         ss >> directive;
//         std::getline(ss, value);
//         location.directives[directive] = trim(value);
//     }

//     server.locations[location_path] = location;
// }

// void Config::log() const {
//     for (std::vector<s_Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
//         const s_Server& server = *it;
//         std::cout << "Server Name: " << server.server_name << ", Listen: " << server.port_to_listen << std::endl;
//         std::cout << "Root: " << server.root << std::endl;
//         std::cout << "Client Max Body Size: " << server.client_max_body_size << std::endl;
//         std::cout << "Error Pages:" << std::endl;
//         for (std::map<int, std::string>::const_iterator err_it = server.error_pages.begin(); err_it != server.error_pages.end(); ++err_it) {
//             std::cout << "  " << err_it->first << " -> " << err_it->second << std::endl;
//         }
//         std::cout << "Locations:" << std::endl;
//         for (std::map<std::string, s_Location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it) {
//             const s_Location& location = loc_it->second;
//             std::cout << "  Location Path: " << loc_it->first << std::endl;
//             for (std::map<std::string, std::string>::const_iterator dir_it = location.directives.begin(); dir_it != location.directives.end(); ++dir_it) {
//                 std::cout << "    " << dir_it->first << " -> " << dir_it->second << std::endl;
//             }
//         }
//         std::cout << "-----------------------" << std::endl;
//     }
// }
