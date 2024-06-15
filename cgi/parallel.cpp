#include <iostream>

#include <unistd.h>

int main()
{
	std::cout << "Status: 200\r\n" << std::flush;
	sleep(5);
	std::cout << "Content-Length: 42\r\n" << std::flush;
	sleep(5);
	std::cout << "Content-Type: text/plain\r\n" << std::flush;
	sleep(5);
	std::cout << "Connection: close\r\n" << std::flush;
	sleep(5);
	std::cout << "\r\n" << std::flush;
	sleep(5);
	std::cout << "01234567890123456789012345678901234567890\n" << std::flush;
	sleep(5);
	return (0);
}
