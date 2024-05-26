/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/27 01:00:13 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

using marvinX::stop_servers;

int	main(int ac, char **av)
{
	Log				log;
	Config			conf(log);
	Cluster			Heart_of_Gold(log);

	log.set_output(&std::cerr, false);
	if (ac != 2)
	{
		log << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	if (DEBUG_MODE) {
		log.set_verbosity(Log::DEBUG);
		log << Log::DEBUG << "Running in debug mode" << std::endl;
	}

	if (conf.configInit(av[1]))
		return EXIT_FAILURE;

	std::vector<Server>	servers = conf.getServers();
	for (std::vector<Server>::iterator it = servers.begin();
	it != servers.end(); ++it) {
		Heart_of_Gold.add_server(*it);
	}
	if (Heart_of_Gold.init_all())
	{
		std::signal(SIGINT, &stop_servers);
		std::signal(SIGTERM, &stop_servers);
		Heart_of_Gold.start();
	} else {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
