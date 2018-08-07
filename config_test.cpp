
#include <memory>
#include <config_file.h>
#include <log.h>

#define show(x) printf("%s=[%s]\n",k,config.get(k,""));
void test(const char*fname)
{
	std::cout<<"----------------------------"<<fname<<"---------------------------\n";
	config_file config;
	config.open(fname);

	config.print(std::cout);

	std::cout<<"-------------------------------------------------------\n";
	for(auto k:config.keys())
		show(k);
	

	log_info("contains(%s)=%d","main.file",config.contains("main.file"));
	log_info("contains(%s)=%d","a",config.contains("a"));
	
	std::cout<<"-------------------------------------------------------\n";
	config.print(std::cout);
}
int main()
{
	test("log.ini");
	test("config.ini");
	return 0;
}

