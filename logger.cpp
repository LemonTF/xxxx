#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <memory>
#include <thread>
#include <log_queue.h>
#include <clock.h>
#include <config_file.h>
#include <sysv_shm.h>
#include <boost/tokenizer.hpp>
#include <boost/utility/string_ref.hpp>
#include <atomic>

#include "log.h"

struct log_file
{
	std::string _fname; //文件名模板 /home/zzj/test.log
	FILE*   _fp;
	long    _min_size;//文件切割的最大的字节数，当达到这个字节数后，下个对齐时间将切换日志文件
	long    _cur_size;//当前文件的大小
	time_t	_last_time;//最后操作文件的时间
	int 	_time_align;//日志文件名对齐的时间，支持 dhm -- 天、小时、分钟

	zclock  _err_clock,_check_dir_clock;

	log_file(const boost::string_ref&fpath,uint32_t min_size=10,int time_align='h')
		:_fname(fpath)
		,_fp(0)
		,_min_size(min_size<<20)
		,_cur_size(0)
	{
		switch(time_align)
		{
			case 'm': case 'M': 
				_time_align=60;
				break;
			case 'd': case 'D': 
				_time_align=3600*24; 
				break;
			case 'H': case 'h': default: 
				_time_align=3600; 
				break;
		}

		time(&_last_time);
		reopen();
	}

	log_file(const log_file&)=delete;
	log_file(log_file&&)=delete;

	inline std::string str_time()
	{
		char buf[128];
		time_t tm=time(0);
		const struct tm*t=localtime(&tm);

		size_t n=
			_time_align==3600?sprintf(buf,"_%02d%02d%02d" , t->tm_mon+1,t->tm_mday,t->tm_hour)
            : _time_align==60  ?sprintf(buf,"_%02d%02d%02d%02d" , t->tm_mon+1,t->tm_mday, t->tm_hour, t->tm_min)
            : sprintf(buf,"_%02d%02d" , t->tm_mon+1,t->tm_mday);

		return std::string(buf,n);
	}

	inline std::string mkname(std::string name)
	{
		int n=name.find_last_of('.');
		name.insert(n,str_time());
		return name;
	}

	int reopen()
	{
		if(_fp) fclose(_fp);

		_fp=fopen(_fname.c_str(),"a+");
		_cur_size=(uint32_t)ftell(_fp);

		_check_dir_clock.reset();
		return 0;
	}

	void close()
	{
		if(_fp)
		{
			fclose(_fp);
			_fp=nullptr;
		}
	
	}

	void put(const char*p,int len)
	{
		time_t ct=time(0);
		if(_cur_size+len>_min_size && _last_time/_time_align != ct/_time_align)
		{
			close();
			rename(_fname.c_str(),mkname(_fname).c_str());
			reopen();
		}

		if(_check_dir_clock.count_ms()>1000)
		{
			if(access(_fname.c_str(),0)==-1)
				reopen();

			_check_dir_clock.reset();
		}

		int rc=fwrite(p,1,len,_fp);
		if(rc<len)
		{
			if(_err_clock.count_ms()>1000)
			{
				std_errno("fwrite:%s",_fname.c_str());
				_err_clock.reset();
			}
		}

		_cur_size+=len;
		_last_time=ct;
	}

	void flush()
	{
		if(_fp) fflush(_fp);
	}

	~log_file()
	{
		fclose(_fp);
	}
};

struct logger
{
	std::shared_ptr<log_queue>   m_queue;
	std::shared_ptr<log_file>    m_file;
	zclock m_clock;
	size_t size_a=0;

	logger(std::shared_ptr<log_queue> shm_queue,std::shared_ptr<log_file> log_file)
		:m_queue(shm_queue)
		,m_file(log_file)
	{
	}

	void run()
	{
		size_t size;
		char   buff[1<<16];

		while((size=m_queue->get(buff,sizeof(buff)))>0)
		{
			m_file->put(buff,size);
			size_a+=size;
		}

		if(m_clock.count_ms()>200 && size_a>0)
		{
			m_file->flush();
			m_clock.reset();
			size_a=0;
		}

		m_queue->keep_alive();
	}

	~logger()
	{
	}
};

static std::shared_ptr<logger> read_config(config_file*f, const char*log_name)
{
	const char* fname=f->get(log_name,"fname","");
	if(strlen(fname)==0)
		return nullptr;

	const char* qsize=f->get(log_name,"queue_size","2048");
	const char* min_size=f->get(log_name,"min_size","10");
	const char* time_align=f->get(log_name,"time_align","m");

	std::unique_ptr<log_queue> queue(new log_queue());

	char path_buf[512];

	realpath(fname,path_buf);
	if(queue->open(path_buf,atoi(qsize)<<10))
	{
		std_error("%s:无法打开共享内存。",log_name);
		return nullptr;
	}
	
	if(!queue->wait_owner())
	{
		std_error("无法取得%s独占共享内存的权限，logger已经启动?",log_name);
		return nullptr;
	}
	
	std::unique_ptr<log_file> file(new log_file(fname, atoi(min_size),time_align[0]));
	std_info("初始化日志%s成功:{文件名=%s, 输出队列大小=%dkB, 最小切换大小=%dMB, 切换对齐时间=%c}",
		log_name,fname,atoi(qsize),atoi(min_size),time_align[0]);

	return std::shared_ptr<logger>(new logger(std::move(queue),std::move(file)));
}

std::atomic<int> g_quit_event(0);
int main(int argc,char*argv[]) 
{ 
	const char*cfg_file="../etc/log.ini";

	for(int i=1;i<argc;i++)
	{
		if(strncmp(argv[i],"-f",2)==0)
		{
			cfg_file=&argv[i][2];
		}
	}

	std::vector<std::shared_ptr<logger>> logs;

	config_file config;
	std_info("初始化配置文件:%s",cfg_file);
	if(config.open(cfg_file))
	{
		std_error("读配置文件%s失败",cfg_file);
		return -1;
	}
	std_info("读取配置文件:%s成功",cfg_file);
	
	std::shared_ptr<logger> logm=read_config(&config,"main");
	if(!logm)
	{
		std_error("无法成功初始化main.log，请检查日志配置文件：%s，日志程序将退出。",cfg_file);
		return -1;
	}
	logs.push_back(logm);
	std_info("初始化主日志main.log成功");

	
	std_info("开始初始化log1-log9，目前系统最多支持9个子日志。");
	char logname[128];
	for(int i=1;i<10;i++)
	{
		sprintf(logname,"log%d",i);
		if(!config.contains(logname,"fname"))
			continue;

		auto logn=read_config(&config,logname);
		if(!logn)
			continue;

		logs.push_back(logn);
	}

	for(int i=0;;i++)
	{
		std::for_each(logs.begin(),logs.end(),[](std::shared_ptr<logger>&log){
			log->run();
		});

		usleep(10000);
	}

	return 0;
} 

