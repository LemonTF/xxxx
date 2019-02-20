#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <thread>
#include <atomic>
#include <vector>

#include <tools.h>
#include <log.h>
#include <log_queue.h>
#include <config_file.h>

struct logp_config
{
	int m_show_thread=1;
	int m_show_level=1;
	int m_show_srcline=1;
	int m_print_stdout=0;
	int m_level=0;
	log_queue*m_queue=nullptr;

	logp_config()
	{
	}

	logp_config(int level,log_queue*queue)
		:m_level(level)
		,m_queue(queue)
	{
	}

	~logp_config()
	{
		delete m_queue;
	}

	int show_level()const{return m_show_level==1;}
	int show_srcline()const{return m_show_srcline==1;}
	int show_thread()const{return m_show_thread==1;}
	int print_stdout()const{return m_print_stdout==1;}

};

std::atomic<int> g_log_inited(-2);
std::vector<std::shared_ptr<logp_config>> g_log_queue;

static inline logp_config*get_lc(int index)
{
	if(index>=0 && index<(int)g_log_queue.size())
		return g_log_queue[index].get();
	
	return nullptr;
}

static inline int level_index(const char*lvls)
{
	static const char* lvl[]={"debug","info","warn","error"};

	for(size_t i=0;i<sizeof(lvl)/sizeof(char*);i++)
	{
		if(strcmp(lvls,lvl[i])==0)
			return i;
	}

	return 0;
}


static std::shared_ptr<logp_config> read_config(config_file*f, const char*log_name)
{
	const char*fname=f->get(log_name,"fname","");
	if(strlen(fname)==0)
		return nullptr;

	const char* qsize=f->get(log_name,"queue_size","2048");

	logp_config lc;

	std::unique_ptr<log_queue> log(new log_queue());

	char path_buf[4096];
	realpath(fname,path_buf);

	if(log->open(path_buf, atoi(qsize)<<10))
		return nullptr;
	
	const char* level=f->get(log_name,"level","info");
	const char* show_level=f->get(log_name,"show_level","1");
	const char* show_srcline=f->get(log_name,"show_srcline","1");
	const char* show_thread=f->get(log_name,"show_thread","1");
	const char* print_stdout=f->get(log_name,"print_stdout","0");

	auto ret=std::make_shared<logp_config>(level_index(level),log.release());

	ret->m_show_level=show_level[0]=='1'?1:0;
	ret->m_show_thread=show_thread[0]=='1'?1:0;
	ret->m_show_srcline=show_srcline[0]=='1'?1:0;
	ret->m_print_stdout=print_stdout[0]=='1'?1:0;

	return ret;
}

int  log_impl_init(const char*cfg_name)
{
	if(g_log_inited!=-2)
		return -1;
	
	++g_log_inited;

	config_file cf;
	if(cf.open(cfg_name))
	{
		--g_log_inited;
		return -1;
	}
	
	g_log_queue.reserve(10);

	auto mlog=read_config(&cf,"main");
	if(mlog)
	{
		g_log_queue.resize(1);
		g_log_queue[0]=mlog;
	}

	char logname[128];
	for(int i=1;i<=9;i++)
	{
		sprintf(logname,"log%d",i);
		if(!cf.contains(logname,"fname"))
			continue;

		auto logn=read_config(&cf,logname);
		if(!logn)
			continue;

		g_log_queue.resize(i+1);
		g_log_queue[i]=logn;
	}

	++g_log_inited;
	return 0;
}

static inline const char*level_str(int level_no)
{
	static const char* lvl[]={"debug","info","warn","error"};
	if(level_no>=(int)(sizeof(lvl)/sizeof(char*)))
		return lvl[0];

	return lvl[level_no];
}

static void print_impl(int id,const char*fname,int line,int level,const char*fmt,va_list arg)
{
	logp_config*lc=get_lc(id);
	if(!lc && id>0)
		return;

	if(lc && level<lc->m_level)
		return;
	
	struct timeval tv;
	gettimeofday(&tv,0);

	struct tm buff;
	const struct tm*t=localtime_r(&tv.tv_sec,&buff);

	char *b0=0;
	int n=1<<10,c;
	do
	{
		b0=(char*)alloca(c=n+1);
		n=snprintf(b0,c,"%d-%02d-%02d %02d:%02d:%02d.%03d " , t->tm_year+1900,t->tm_mon+1,t->tm_mday,
				t->tm_hour,t->tm_min,t->tm_sec,(int)(tv.tv_usec/1000));

		if((!lc || lc->show_level()) && c>n)
		{
			n+=snprintf(b0+n,c-n,"[%s]", level_str(level));
		}

		if(lc && lc->show_thread() && c>n)
		{
			std::hash<std::thread::id> hasht;
			n+=snprintf(b0+n,c-n,"[%04X]", (unsigned)(hasht(std::this_thread::get_id())&0XFFFF));
		}

		if(lc && lc->show_srcline() && c>n)
		{
			n+=snprintf(b0+n,c-n,"[%s:%d]", fname,line);
		}

		if(c>n)
		{
			n+=snprintf(b0+n,c-n," %s\n", fmt);
		}
	}while(n>c);

	va_list tmp;
	va_copy(tmp,arg);

	n=4<<10;
	char *b1=0;
	do
	{
		if(b1)
		{
			free(b1);
			va_copy(arg,tmp);
		}
		b1=(char*)malloc(c=n+1);
		n=vsnprintf(b1,c,b0,arg);

	}while(n>c);

	if(!lc)
	{
		printf("%s",b1);
		fflush(stdout);
	}
	else
	{
		if(lc->print_stdout())
		{
			printf("%s",b1);
			fflush(stdout);
		}

		lc->m_queue->put(b1,n);
	}
	if(b1)free(b1);
}

void log_impl_print_errno(int id,const char*fname,int line,int level,const char*fmt,...)
{
	char buff[1024];
	int e=errno;
	sprintf(buff,"errno=%d,errinfo=%s,addinfo=%s",e,strerror(e),fmt);

	va_list ap;
	va_start(ap, fmt);
	print_impl(id,fname,line,level,buff,ap);
	va_end(ap);
}

void log_impl_print	(int id,const char*fname,int line,int level,const char*fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	print_impl(id,fname,line,level,fmt,ap);
	va_end(ap);
}

void log_impl_binary(int log_id,const char*fname,int line,const char*addmsg,const char*d,int len)
{
	std::string bin=format_bin2(d,len);
	log_impl_print(log_id,fname,line,1,"%s,len=%d\n%s",addmsg,len,bin.c_str());
}

