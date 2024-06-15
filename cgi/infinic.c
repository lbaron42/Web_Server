#include <unistd.h>

int main(int ac, char **av, char **envp)
{
	if (!envp)
		return 1;
	write(1, "Stat", 4);
	while (ac)
	{
		if (!av)
			sleep(2);
		else if (*av)
			++av;
		else
			--av;
		sleep(2);
	}
	return (0);
}
