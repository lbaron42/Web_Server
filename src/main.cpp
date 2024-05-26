/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/26 22:50:08 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

using marvinX::stop_servers;

// static std::vector<Server> create_mock_servers(Log &log, int n_of_servers);

int	main(int ac, char **av)
{
	Log				log;
	Cluster			Heart_of_Gold(log);
	Config			conf(log);

	log.set_output(&std::cerr, false);
	if (ac != 2)
	{
		log << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	if (DEBUG_MODE)
		log.set_verbosity(Log::DEBUG);
	log << Log::DEBUG << "Running in debug mode" << std::endl;

	if (conf.configInit(av[1]))
	{
		return EXIT_FAILURE;
	}
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
	}
	return EXIT_SUCCESS;
}
