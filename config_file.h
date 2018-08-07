#ifndef __CONFIG_FILE_HPP_
#define __CONFIG_FILE_HPP_
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string.h>

struct cchar_cmp
{
	bool operator()(const char*l,const char*r)const
	{
		return strcmp(l,r)<0;
	}
};

struct config_file
{
private:
	std::vector<char> _buf;
	std::map<const char*,const char*,cchar_cmp> _map;
	config_file(const config_file&)=delete;
public:
	config_file (){}
	~config_file(){}

	int open(const char*fname);

	const char* get(const char*key,const char*v);
	const char* get(const char*sec,const char*key,const char*v);

	std::set<const char*> keys();

	bool contains(const char*k);
	bool contains(const char*sec,const char*k);

	void print(std::ostream&o);
};
#endif

