/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/22 20:35:32 by lbaron           ###   ########.fr       */
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


struct ServerData
{
	struct Address
	{
		std::string				ip;
		std::string				port;
	};
	struct Location
	{
		std::string				_location_path;
		std::string				alias;
		std::string				index;
		std::string				allowed_methods;
		bool 					is_redirection;
	};

	std::vector<Address>		addresses;
	std::vector<std::string>	hostname;
	std::string					root;
	std::string					index;
	std::vector<Location>		locations;
	size_t						client_max_body_size;
	bool						directory_listing;
};


class Config {
public:
	int configInit(const std::string& argv1);
	void log() const;
	// const std::vector<s_Server>& getServers() const;

private:
	std::vector<ServerData> servers;
	void getAddress(std::string line, ServerData *current);
};

#endif // CONFIG_HPP

