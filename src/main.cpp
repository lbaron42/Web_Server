/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/23 17:18:02 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

using marvinX::stop_server;

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	Log				log;
	log.set_output(&std::cerr, false);
	if (DEBUG_MODE)
		log.set_verbosity(Log::DEBUG);
	log << Log::DEBUG << "Running in debug mode" << std::endl;

	Config	conf;
	if (conf.configInit(av[1]))
	{
		std::cerr << "Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	// std::cout << conf;

	Server serv("MarvinX", "8080", log);
	if (serv.initialize())
	{
		std::signal(SIGINT, &stop_server);
		std::signal(SIGTERM, &stop_server);
		serv.start();
	}
	return EXIT_SUCCESS;
}

