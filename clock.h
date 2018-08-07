#ifndef _clock_hpp_zzj_
#define _clock_hpp_zzj_

#include <time.h>

#ifdef _MSC_VER
struct zclock
{
	unsigned m_start;
	zclock()
	{
		m_start=GetTickCount ();
	}

	long long count_us() const
	{
		return 1000L*count_ms();
	}

	unsigned count_ms()const
	{
		return (GetTickCount()-m_start);
	}

	void reset()
	{
		m_start=GetTickCount ();
	}
	
	long long pin_us()
	{
		return 1000L*pin_ms();
	}

	unsigned pin_ms()
	{
		unsigned rc=count_ms();
		m_start=GetTickCount();
		return rc;
	}
};
#else
struct zclock
{
	struct timespec m_start;

	zclock()
	{
		clock_gettime(CLOCK_MONOTONIC,&m_start);
	}

	void reset()
	{
		clock_gettime(CLOCK_MONOTONIC,&m_start);
	}

	long long count_us() const
	{
		struct timespec m_tmp;
		clock_gettime(CLOCK_MONOTONIC,&m_tmp);

		long long ret=(m_tmp.tv_sec-m_start.tv_sec)*1000000;
		ret+=(m_tmp.tv_nsec-m_start.tv_nsec)/1000;

		return ret;
	}

	unsigned count_ms()const
	{
		return count_us()/1000;	
	}
	
	long long pin_us()
	{
		struct timespec m_tmp;
		clock_gettime(CLOCK_MONOTONIC,&m_tmp);

		long long ret=(m_tmp.tv_sec-m_start.tv_sec)*1000000;
		ret+=(m_tmp.tv_nsec-m_start.tv_nsec)/1000;

		m_start=m_tmp;

		return ret;
	}

	unsigned pin_ms()
	{
		return pin_us()/1000;
	}
};
#endif
#endif
