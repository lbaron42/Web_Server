/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 12:06:01 by mcutura          ###   ########.fr       */
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

	std::ifstream	config(av[1]);
	if (!config.is_open())
	{
		log << Log::ERROR << "Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	// parse and validate config file
	config.close();

	Server serv("MarvinX", "8080", log);
	if (serv.initialize())
	{
		std::signal(SIGINT, &stop_server);
		std::signal(SIGTERM, &stop_server);
		serv.start();
	}
	return EXIT_SUCCESS;
}
