#include <string>
#include <fstream>
#include <iterator>

#include <log.h>
#include <config_file.h>

static void trim(char*b,char*e)
{
	char*e1=e;
	for(;e1>b;--e1)
	{
		if(!std::isspace(e1[-1]))
			break;
	}

	char*b1=b;
	for(;b1<e1;++b1)
	{
		if(!std::isspace(*b1))
			break;
	}

	memmove(b,b1,e1-b1);
	b[e1-b1]=0;
}

int config_file::open(const char*fname)
{
	_buf.reserve(1<<10);
	std::ifstream f(fname,std::ios_base::binary);

	if(!f)
	{
		log_errno("can't open the inifile:%s",fname);
		return -1;
	}

	f.unsetf(std::ios::skipws);

	std::copy(std::istream_iterator<char>(f),std::istream_iterator<char>(), std::back_inserter(_buf));
	_buf.push_back('\n');
	_buf.push_back('0');

	char*s=&_buf[0];
	char*e=&_buf.back();
	for(;s<e;)
	{
		char*p=strchr(s,'\n');
		if(p==0)
			break;
		*p=0;
		if(p==s)
		{
			s=p+1;
			continue;
		}

		char*v=strchr(s,'=');
		if(v==0)
		{
			s=p+1;
			continue;
		}
		char*r=strchr(v+1,'#');
		if(r) *r=0;
		else r=p;

		char*k=s;
		*p=0;
		*v=0;
			
		trim(k,v);
		v++;
		trim(v,r);

		_map.insert(std::make_pair(k,v));

		s=p+1;
	}

	return 0;
}

int config_file::get(const char*key,int default_v)const
{
	auto it=_map.find(key);

	return it==_map.end()
			?  (log_info("config param: %s=%d(default)",key,default_v),default_v) 
			:  (log_info("config param: %s=%s",key,it->second), atoi(it->second));
}

int config_file::get(const char*sec,const char*key,int default_v)const
{
	char k[512];
	sprintf(k,"%s.%s",sec,key);

	return get(k,default_v);
}

double config_file::get(const char*key,double default_v)const
{
	auto it=_map.find(key);

	return it==_map.end()
			?  (log_info("config param: %s=%lf(default)",key,default_v),default_v) 
			:  (log_info("config param: %s=%s",key,it->second), atof(it->second));
}

double config_file::get(const char*sec,const char*key,double default_v)const
{
	char k[512];
	sprintf(k,"%s.%s",sec,key);

	return get(k,default_v);
}

const char* config_file::get(const char*sec,const char*key,const char*v)const
{
	char k[512];
	sprintf(k,"%s.%s",sec,key);
	return get(k,v);
#if 0
	auto it=_map.find(k);
	return it==_map.end()
			?  get(k,v)
			:  (log_info("config param: %s=%s",k,it->second),it->second);
#endif
}

const char* config_file::get(const char*key,const char*defv)const
{
	auto it=_map.find(key);

	return it==_map.end()
			?  (log_info("config param: %s=%s(default)",key,defv),defv) 
			:  (log_info("config param: %s=%s",key,it->second), it->second);
}

std::set<const char*> config_file::keys()const
{
	std::set<const char*> ret;
	for(auto it:_map)
		ret.insert(it.first);

	return std::move(ret);
}

bool config_file::contains(const char*k)const
{
	return _map.find(k)!=_map.end();
}

bool config_file::contains(const char*sec,const char*k)const
{
	char key[512];
	snprintf(key,255,"%s.%s",sec,k);
	return contains(key);
}

void config_file::print(std::ostream&o)
{
	for(auto &m:_map)
		o<<m.first<<"="<<m.second<<"\n";
	o<<std::ends;
}

