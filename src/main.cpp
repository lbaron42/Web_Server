/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 08:22:31 by mcutura           #+#    #+#             */
/*   Updated: 2024/05/20 01:15:24 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

using marvinX::stop_servers;

static std::vector<Server> create_mock_servers(Log &log, int n_of_servers);

int	main(int ac, char **av)
{
	Log				log;
	Cluster			Heart_of_Gold(log);
	Config			conf;

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
		log << Log::ERROR << "Couldn't open config file" << std::endl;
		return EXIT_FAILURE;
	}

	/* DELETE BLOCK
		Remove after Config Server creation is done
	*/
	std::vector<Server>	example = create_mock_servers(log, 2);
	for (std::vector<Server>::iterator it = example.begin();
	it != example.end(); ++it)
		Heart_of_Gold.add_server(*it);
	/* END DELETE BLOCK */

	if (Heart_of_Gold.init_all())
	{
		std::signal(SIGINT, &stop_servers);
		std::signal(SIGTERM, &stop_servers);
		Heart_of_Gold.start();
	}
	return EXIT_SUCCESS;
}

std::vector<Server> create_mock_servers(Log &log, int n_of_servers)
{
	std::vector<Server>	mockservs;

	for (int i = 0; i < n_of_servers; ++i)
	{
		std::stringstream	port;
		std::stringstream	hostname;

		port << (i + 8080);
		ServerData	sd;
		sd.root = "extra/webspace/mc-putchar.github.io";
		sd.index = "index.html";
		sd.allowed_methods = static_cast<Request::e_method>(
				Request::HEAD | Request::GET | Request::POST);
		sd.directory_listing = true;
		ServerData::Address		addr;
		addr.ip = "127.0.0.1";
		addr.port = port.str();
		sd.address.push_back(addr);
		hostname << "marvinx" << i << ".42.fr";
		sd.hostname.push_back(hostname.str());
		hostname.clear();
		hostname.str(std::string());
		hostname << "www." << "marvinx" << i << ".42.fr";
		sd.hostname.push_back(hostname.str());
		mockservs.push_back(Server(sd, log));
	}
	return mockservs;
}
