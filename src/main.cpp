/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/15 11:09:49 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "Config.hpp"
#include "Server.hpp"

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: webserv [CONFIGURATION FILE]" << std::endl;
		return EXIT_FAILURE;
	}
	std::ifstream	config_file(av[1]);
	if (!config_file.is_open())
	{
		std::cerr << "ERROR: Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}
	Config	conf;
	conf.configInit(av[1]);
	config_file.close();

	Server serv;
	if (serv.initialize())
		serv.start();
	return EXIT_SUCCESS;
}
