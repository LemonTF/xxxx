#include <vector>
#include <iostream>
#include <algorithm>
#include <tools.h>

int main()
{
	std::vector<char> b(2003,0);
	std::generate(b.begin(),b.end(),[](){return rand()&0xFF;});

	for(int i=0;i<10000;i++)
		std::cout<<format_bin2(&b.front(),b.size())<<std::endl;

	return 0;
}
