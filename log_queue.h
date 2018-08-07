#ifndef __zlog_ipc_hpp__
#define __zlog_ipc_hpp__

#include <sysv_shm.h>

struct log_header;
struct log_queue
{
private:
	sysv_shm  _shm;
	log_header*_base;
public:
	log_queue();
	~log_queue();

	int open(const char*name,size_t queue_size);

	void put(const char*s,int len);
	int  get(char*s,int len);

	bool wait_owner();
	void keep_alive();
};
#endif

