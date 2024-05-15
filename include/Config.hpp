/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 20:04:14 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/14 23:05:20 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <string>
# include <map>
# include <fstream>
# include <vector>

struct LocationContext {
    std::map<std::string, std::string> settings;
};

struct ServerContext {
    std::map<std::string, std::string> settings;
    std::map<std::string, LocationContext> locations;
};

struct HttpContext {
    std::map<std::string, std::string> settings;
    std::map<std::string, ServerContext> servers;
};

struct EventsContext {
    std::map<std::string, std::string> settings;
};

struct MainContext {
    std::map<std::string, std::string> settings;
    EventsContext events;
    HttpContext http;
};

class Config
{

	public:

		Config();
		Config( Config const & src );
		~Config();

		Config &		operator=( Config const & rhs );


		// to be implemented!!
		void configInit(std::string argv1);

	private:
		MainContext _mainContext;


};

std::ostream &			operator<<( std::ostream & o, Config const & i );

#endif /* ********************************************************** CONFIG_H */