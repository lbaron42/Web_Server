/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/17 12:30:04 by lbaron           ###   ########.fr       */
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

    s_Server(std::string _name, int _index) : index(_index), index_name(_name){}
    int index;
    std::string index_name;
    std::map<std::string, std::string> servKeywords;
    std::map<std::string, s_Location> locations;
};
class Config {
public:
    Config();
    int configInit(const std::string& argv1);
    /* print the servers and its respective keyWords and locations
     use only for debug purposes only */
    void log() const;
    /*Retrive the vector holding the s_Server structs, for usage reference 
    you can look in to the log(); implementation*/
    const std::vector<s_Server>& getServers() const;

private:
    std::vector<s_Server> servers;
    std::vector<std::string> keyWords;
};

#endif // CONFIG_HPP

