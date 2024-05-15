/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/14 10:52:33 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "Server.hpp"

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

	Server serv;
	if (serv.initialize())
		serv.start();
	return EXIT_SUCCESS;
}