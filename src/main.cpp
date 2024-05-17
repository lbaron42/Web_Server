/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/17 12:21:37 by lbaron           ###   ########.fr       */
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
	Config conf;
    if (conf.configInit(av[1]) != 0)
    {
        return EXIT_FAILURE;
    }
	conf.log();

	Server serv;
	if (serv.initialize())
		serv.start();
	return EXIT_SUCCESS;
}
