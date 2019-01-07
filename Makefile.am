
AM_SOURCES=clock.h log.cpp log.h log_queue.cpp log_queue.h ztomic.h tools.cpp tools.h sysv_shm.cpp sysv_shm.h config_file.cpp config_file.h
AM_CPPFLAGS=-Wall -pthread -m64 -std=c++11 -fPIC
AM_LDFLAGS= -Wall -pthread -m64 -std=c++11 -fPIC

lib_LTLIBRARIES=libzlog.la
bin_PROGRAMS=logger 
noinst_PROGRAMS=test shmtest fmttest cct

AM_CPPFLAGS+=
AM_LDFLAGS+=

include_HEADERS=log.h config_file.h

libzlog_la_SOURCES=${AM_SOURCES}
libzlog_la_CPPFLAGS=${AM_CPPFLAGS}
libzlog_la_LDFLAGS=${AM_LDFLAGS} -shared -lrt

logger_SOURCES=logger.cpp config_file.cpp
logger_LDFLAGS=${AM_LDFLAGS} -lrt -L. -lzlog 

test_SOURCES=test.cpp 
test_LDFLAGS=${AM_LDFLAGS} -lrt -L.  -ldl -lzlog

shmtest_SOURCES=shmtest.cpp tools.cpp sysv_shm.cpp
shmtest_LDFLAGS=${AM_LDFLAGS} -lrt 

fmttest_SOURCES=fmttest.cpp tools.cpp 
fmttest_LDFLAGS=${AM_LDFLAGS} -lrt 

cct_SOURCES=config_test.cpp config_file.cpp
cct_LDFLAGS=${AM_LDFLAGS} -lrt -L. -lzlog

DEFS= 
EXTRA_DIST= clock.h config_file.cpp config_file.h config_test.cpp log.cpp logger.cpp log.h \
		    log_queue.cpp log_queue.h test.cpp ztomic.h

