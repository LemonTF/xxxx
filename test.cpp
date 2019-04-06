#include <unistd.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <log.h>
#include <dirent.h>
#include <string.h>
#include <fnmatch.h>

void dir_clean(const char*dir,const char*pattern,int count)
{
	struct dirent **namelist;
	int n = scandir(dir, &namelist, NULL, alphasort);

	if (n < 0)
		return;

	char fname[512];
	strcpy(fname,dir);
	strcat(fname,"/");
	char*p=&fname[strlen(fname)];

	for (int i=n-1;i>=0;i--)
	{
		if(0==fnmatch(pattern,namelist[i]->d_name,0) && count--<=0)
		{
			strcpy(p,namelist[i]->d_name);
			remove(fname);
			printf("remove file:%s\n",fname);
		}
		free(namelist[i]);
	}
	free(namelist);
}

int main() 
{ 
	dir_clean("/home/zzj/ya-src/dist/log","yals_*.log",3);
	dir_clean("../dist/log","yals_*.log",3);
	
	log_init("../etc/log.ini");

	std::string s(100,'1');
	for(int i=0;;i++)
	{
		log_info("%s",s.c_str());
		usleep(10);
	}

	std::this_thread::yield();
	return 0;
} 

