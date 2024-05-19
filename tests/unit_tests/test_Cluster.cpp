#include <csignal>
#include <cstdlib>
#include <vector>
#include "Cluster.hpp"
#include "Server.hpp"

namespace {
	static char const	*tests_log = "tests/unit_tests/tests.log";
}

static std::vector<Server> create_mock_servers(Log &log, int n_of_servers);

int main()
{
	Log			log;
	Cluster		clustest(log);

	log.set_output(new std::ofstream(tests_log, \
		std::ios_base::ate | std::ios_base::app), true);
	log.set_verbosity(Log::DEBUG);
	std::vector<Server>	server_list = create_mock_servers(log, 100);
	for (std::vector<Server>::iterator it = server_list.begin();
	it != server_list.end(); ++it) {
		clustest.add_server(*it);
	}
	if (clustest.init_all())
	{
		std::signal(SIGINT, &marvinX::stop_servers);
		std::signal(SIGTERM, &marvinX::stop_servers);
		clustest.start();
	} else {
		log << Log::ERROR << "Failed initializing servers" << std::endl;
		return EXIT_FAILURE;
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
		ServerData	sd("/", "index.html");
		sd.address.push_back(ServerData::Address("127.0.0.1", port.str()));
		hostname << "MarvinX" << i;
		sd.hostname.push_back(hostname.str());
		mockservs.push_back(Server(sd, log));
	}
	return mockservs;
}
