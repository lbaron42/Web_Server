/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/16 17:23:03 by lbaron           ###   ########.fr       */
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
struct s_Location {
    std::map<std::string, std::string> directives;
};

struct s_Server {

    s_Server(std::string _name, int _index){
        _name = server_name;
        _index = index;
    }
    int index;
    std::string port_to_listen;
    std::string server_name;
    std::string root;
    std::map<int, std::string> error_pages;
    std::string client_max_body_size;
    std::map<std::string, s_Location> locations;
};

class Config {
public:
    int configInit(const std::string& filename);
    void log() const;

private:
    std::vector<s_Server> servers;
    // void parseServer(std::ifstream& config_file);
    // void parseLocation(std::ifstream& config_file, s_Server& server, const std::string& location_path);
    // std::string trim(const std::string& str) const;
};

#endif // CONFIG_HPP

