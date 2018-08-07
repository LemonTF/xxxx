#include <boost/stacktrace.hpp>
#include <iostream>


void test(int frame)
{
	if(frame--<=0)
	{
		std::cout<< boost::stacktrace::stacktrace();
	}
	else
	{
		test(frame);
	}
}

int main()
{
	test(5);

	return 0;
}
