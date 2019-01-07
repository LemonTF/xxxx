
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include <thread>
#include <ztomic.h>

#include <memory.h>
#include <clock.h>
#include <tools.h>
#include <log.h>
#include <sysv_shm.h>
#include <log_queue.h>

struct log_header
{
	uint64_t _size;
	uint64_t _mask;
	uint64_t _pad1[6];

	uint64_t _gpos;//读取指针
	uint64_t _pad2[7];
	uint64_t _npos;//申请指针
	uint64_t _pad3[7];
	uint64_t _cpos;//提交指针
	uint64_t _pad4[7];
	pid_t    _owner;
	uint64_t _tick;
	uint64_t _pad5[6];
	char     p[0];

	inline void write(uint64_t pos,const char*s,int len)
	{
		int wp=index(pos);
		int ep=index(pos+len);

		if(wp<ep)
		{
			memcpy(&p[wp],s,len);
		}
		else
		{
			memcpy(&p[wp],s,_size-wp);
			memcpy(&p[0],s+_size-wp,ep);
		}
	}

	inline void read(uint64_t pos,char*b,int len)
	{
		int rp=index(pos);
		int ep=index(pos+len);

		if(rp<ep)
		{
			memcpy(b,&p[rp],len);
		}
		else
		{
			memcpy(b,&p[rp],_size-rp);
			memcpy(b+_size-rp,&p[0],ep);
		}
	}

	inline uint32_t yield(uint32_t n)
	{
		if(n<200)
		{
		}
		else if(n<1000)
		{
			std::this_thread::yield();
		}
		else if(n<2000)
		{
			usleep(1);
		}
		else
		{
			usleep(100);
		}

		return n+1;
	}

	inline int index(uint64_t pos)
	{
		return (int)(pos&_mask);
	}

	inline int  mini(int i1,int i2)
	{
		return i1<i2?i1:i2;
	}


//-----------|-------------|-----------------|>>>>>>>>
//           g             c                 n
	void print(const char*s,int len)
	{
		uint64_t npos=ztomic::load(&_npos);
		uint32_t n=0;
		//申请空余的空间
		for(;;)
		{
			if(ztomic::cas(&_npos,&npos,npos+len))
				break;

			n=yield(n);
		}

		n=0;
		zclock c;
		uint64_t tm_all=0;
		//检查回绕，避免缓冲区重写
		for(uint32_t i=0;;i++)
		{
			uint64_t gpos=ztomic::load(&_gpos);
			if(gpos+_size>=npos+len)
				break;

			if((n=yield(n))>4000 && c.count_ms()>500)
			{
				tm_all+=c.pin_ms();
				std_error("log_queue::print已经阻塞了%ldms，请检查日志输出进程是否已经打开!\n",tm_all);
			}
		}

		write(npos,s,len);

		n=0;
		c.reset();
		for(uint32_t i=0;;i++)
		{
//			uint64_t cpos=ztomic::load(&_cpos);
			uint64_t n=npos;
			if(ztomic::cas(&_cpos,&n,n+len))
				break;

			if(n>=npos)
				break;

			if(n<npos && i>500 && c.count_ms()>200)//超时的话，直接设置提交指针
			{
				ztomic::store(&_cpos,npos+len);
				break;
			}

			n=yield(n);
		}
	}
//-----------|-------------|-----------------|>>>>>>>>
//           g             c                 n

	int  get(char*s,int len)
	{
		uint64_t cpos=ztomic::load(&_cpos);
		if(_gpos<cpos)
		{
			int rc=mini(len,cpos-_gpos);
			read(_gpos,s,rc);
			ztomic::store(&_gpos,_gpos+rc);
			return rc;
		}

		return 0;
	}

	bool wait_owner()
	{
		pid_t owner=ztomic::load(&_owner);
		if(owner==0)
		{
			ztomic::store(&_owner,getpid());
			set_live();
			return true;
		}

		if(_owner==getpid())
		{
			set_live();
			return true;
		}

		uint64_t age0=age();
		for(int i=0;i<1000;i++)
		{
			if(age0>age())
				return false;

			usleep(1000);
		}

		ztomic::store(&_owner,getpid());
		set_live();
		return true;
	}

	//设置最后访问的时间戳
	void set_live()
	{
		struct timespec m_tmp;
		clock_gettime(CLOCK_MONOTONIC,&m_tmp);

		ztomic::store((uint64_t*)&_tick,*(uint64_t*)&m_tmp);
	}

	//当前时间与存储时间戳的差
	uint64_t age()const
	{
		struct timespec m_tmp;
		clock_gettime(CLOCK_MONOTONIC,&m_tmp);
		uint64_t start0=ztomic::load((uint64_t*)&_tick);
		struct timespec start=*(struct timespec*)&start0;

		long long ret=(m_tmp.tv_sec-start.tv_sec)*1000000;
		ret+=(m_tmp.tv_nsec-start.tv_nsec)/1000;
		return ret/1000;
	}
};

log_queue::log_queue()
{
	_base=nullptr;
}

log_queue::~log_queue()
{
}

int log_queue::open(const char*name,size_t queue_size)
{
	size_t size=1<<20;
	while(size<queue_size)
		size<<=1;


	std_info("share memory fname:%s",name);
	if(_shm.open(name,size+sizeof(log_header))<0)
		return -1;

	_base=(log_header*) _shm.ptr();

	_base->_size=size;
	_base->_mask=size-1;

	return 0;
}

//-----------|-------------|-----------------|>>>>>>>>
//           g             c                 n

void log_queue::put(const char*s,int len)
{
	_base->print(s,len);
}

int  log_queue::get(char*s,int len)
{
	return _base->get(s,len);
}

void log_queue::keep_alive()
{
	return _base->set_live();
}

bool log_queue::wait_owner()
{
	return _base->wait_owner();
}



