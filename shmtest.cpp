#include <unistd.h>

#include <errno.h>
#include <sys/shm.h>
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory.h>
#include <clock.h>
#include <tools.h>
#include <sysv_shm.h>

int main()
{
	std::vector<char> b(202,0);
	std::generate(b.begin(),b.end(),[](){return rand()&0xFF;});
	std::cout<<format_bin(&b.front(),b.size())<<std::endl;

	return 0;




	sysv_shm t;

	t.open("TEST",1);

	if(t.num_attach()>1)
	{
		printf("nattach=%d\n",t.num_attach());
		return 0;
	}

	while(true)
	{
		printf("nattach=%d\n",t.num_attach());

		sleep(1);
	}

	return 0;
}
