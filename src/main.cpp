/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/16 17:16:09 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "Server.hpp"

using marvinX::stop_server;

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	std::ifstream	config(av[1]);
	if (!config.is_open())
	{
		std::cerr << "ERROR: Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	// parse and validate config file
	config.close();

	Server serv("MarvinX", "8080");
	if (serv.initialize())
	{
		std::signal(SIGINT, &stop_server);
		std::signal(SIGTERM, &stop_server);
		serv.start();
	}
	return EXIT_SUCCESS;
}