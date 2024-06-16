#include <stdio.h>

int main(int ac, char **av, char **envp)
{
	if (ac > 1)
		(void)av;
	if (!envp)
		return 1;
	printf("Status: 200 OK\n");
	printf("Content-Type: text/plain\n\n");
	while (*envp) {
		printf("%s\n", *envp);
		++envp;
	}
	return (0);
}
