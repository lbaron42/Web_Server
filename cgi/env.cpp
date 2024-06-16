#include <algorithm>
#include <cctype>
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

int main(int ac, char **av, char **envp)
{
	if (ac > 1)
		(void)av;
	if (!envp)
		return 1;
	std::map<std::string, std::string>	env;
	while (*envp) {
		std::string	tmp(*envp++);
		std::string::size_type	pos(tmp.find("="));
		if (pos == std::string::npos)
			continue;
		env.insert(std::make_pair(
			tmp.substr(0, pos),
			tmp.substr(pos + 1)));
	}
	if (env.empty())
		return 1;
	std::map<std::string, std::string>::const_iterator method;
	method = env.find("REQUEST_METHOD");
	if (method == env.end()) {
		std::cout << "No request method found\r\n"
			<< "Status: 400\r\n" << std::endl;
		return 1;
	}
	if (method->second.compare("GET")) {
		std::cout << "Status: 405\r\n"
			<< "Method: " << method->second << std::endl;
		return 1;
	}
	std::stringstream	ss;
	for (std::map<std::string, std::string>::const_iterator it = env.begin();
	it != env.end(); ++it) {
		ss << it->first << ": " << it->second << "\r\n";
	}
	ss.seekg(0, std::ios::end);
	std::stringstream::pos_type	body_size = ss.tellg();
	ss.seekg(0, std::ios::beg);
	std::cout << "Content-Type: text/plain" << "\r\n"
		<< "Content-Length: " << body_size << "\r\n"
		<< "\r\n"
		<< ss.str() << std::flush;
	return 0;
}
