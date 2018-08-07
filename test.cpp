#include <unistd.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <log.h>

int main() 
{ 
	log_init("log.ini");

	std::string s(1<<20,'1');
	for(int i=0;;i++)
	{
		log_info("%s","123456789");
		usleep(10);
//		std::cin.get();
	}

	std::this_thread::yield();
	return 0;
} 

