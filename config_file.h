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


	int get(const char*key,int default_v)const;
	int get(const char*sec,const char*key,int default_v)const;

	double get(const char*key,double default_v)const;
	double get(const char*sec,const char*key,double default_v)const;

	const char* get(const char*key,const char*default_v)const;
	const char* get(const char*sec,const char*key,const char*default_v)const;

	std::set<const char*> keys()const;

	bool contains(const char*k)const ;
	bool contains(const char*sec,const char*k)const;

	void print(std::ostream&o);
};
#endif

