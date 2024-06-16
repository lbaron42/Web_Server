/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/06/09 22:14:03 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

using marvinX::stop_servers;

int	main(int ac, char **av)
{
	Log			log;
	Config		conf(log);
	Cluster		Heart_of_Gold(log);

	log.set_output(&std::cerr, false);
	if (ac > 2)
	{
		log << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	if (DEBUG_MODE) {
		log.set_verbosity(Log::DEBUG);
		log << Log::DEBUG << "Running in debug mode" << std::endl;
	}
	char const	*config_path = (ac == 2 ? av[1] : "config/default.conf");
	if (conf.configInit(config_path))
		return EXIT_FAILURE;
	std::vector<Server>	servers = conf.getServers();
	if (servers.empty()) {
		log << Log::ERROR << "No valid servers defined. Exiting" << std::endl;
		return EXIT_FAILURE;
	}
	for (std::vector<Server>::iterator it = servers.begin();
	it != servers.end(); ++it)
		Heart_of_Gold.add_server(*it);
	if (!Heart_of_Gold.init_all())
		return EXIT_FAILURE;
	std::signal(SIGINT, &stop_servers);
	std::signal(SIGTERM, &stop_servers);
	Heart_of_Gold.start();
	return EXIT_SUCCESS;
}
