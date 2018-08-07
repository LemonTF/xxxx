#ifndef __SYSV_SHM_HPP_
#define __SYSV_SHM_HPP_
struct sysv_shm
{
private:
	int   _id;
	void* _base;
public:
	sysv_shm();
	~sysv_shm();

	int open(const char*name,size_t size);
	char*ptr();
	int num_attach();
	void close();
	int destroy();
};

#endif

