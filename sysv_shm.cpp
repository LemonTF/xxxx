#include <unistd.h>
#include <sys/shm.h>

#include <exception>
#include <stdio.h>
#include <string.h>

#include <tools.h>
#include <sysv_shm.h>

sysv_shm::sysv_shm()
	:_id(-1)
	,_base(0)
{
}

char*sysv_shm::ptr()
{
	return (char*)_base;
}

int sysv_shm::open(const char*name,size_t size)
{
	key_t key=hash(name,strlen(name));
	if((_id=shmget(key, size, IPC_CREAT|0644)) == (-1)) 
	{
		perror("shmget");
		return -1;
	}

	if((_base=shmat(_id,0,0))==(void*)(-1))
	{
		perror("shmat");
		return -1;
	}

	return 0;
}

int sysv_shm::num_attach()
{
	struct shmid_ds sd={0};

	if(shmctl(_id,IPC_STAT,&sd)<0)
	{
		perror("shmctl");
		return -1;
	}

	return (int)sd.shm_nattch;
}

int sysv_shm::destroy()
{
	if(shmctl(_id,IPC_RMID,0)<0)
	{
		perror("shmctl");
		return -1;
	}

	return 0;
}

void sysv_shm::close()
{
	int num=num_attach();
	if(_base) 
		shmdt(_base);

	if(num==1) 
		destroy();
}

sysv_shm::~sysv_shm()
{
	close();
}

