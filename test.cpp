#include <unistd.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <log.h>

int main() 
{ 
	log_init("../etc/log.ini");

	std::string s(100,'1');
	for(int i=0;;i++)
	{
		log_info("%s",s.c_str());
	}

	std::this_thread::yield();
	return 0;
} 

