#include <algorithm>
#include <string>
#include <map>

#include <unistd.h>

int main(int ac, char **av, char **envp)
{
	if (ac > 1)
		(void)av;
	if (!envp || !*envp)
		return 1;
	std::map<std::string, std::string>	env;
	while (*envp) {
		std::string	tmp(*envp);
		std::string::size_type	pos(tmp.find("="));
		if (pos == std::string::npos)
			continue;
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
		env.insert(std::make_pair(
			tmp.substr(0, pos),
			tmp.substr(pos + 1)));
	}
	// Many work, very time, loopity infinite
	while (1) {
		for (std::map<std::string, std::string>::iterator it = env.begin();
		it != env.end(); ++it)
			std::reverse(it->second.begin(), it->second.end());
		usleep(1000);
	}
	return 0;
}
