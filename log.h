#ifndef __ZLOG_HPP__
#define __ZLOG_HPP__

extern int log_impl_init(const char*config_file_name);
extern void log_impl_print(int log_id,const char*fname,int line,int level,const char*fmt,...);
extern void log_impl_print_errno(int log_id,const char*fname,int line,int level,const char*fmt,...);
extern void log_impl_binary(int log_id,const char*fname,int line,const char*add_msg,const char*d,int len);

#define log_init(cfg_name)log_impl_init(cfg_name)

#define logn_debug(id,fmt,...)log_impl_print(id,__FILE__,__LINE__,0,fmt, ##__VA_ARGS__ )
#define logn_info(id,fmt,...)log_impl_print(id,__FILE__,__LINE__,1,fmt, ##__VA_ARGS__ )
#define logn_warn(id,fmt,...)log_impl_print(id,__FILE__,__LINE__,2,fmt, ##__VA_ARGS__ )
#define logn_error(id,fmt,...)log_impl_print(id,__FILE__,__LINE__,3,fmt, ##__VA_ARGS__ )
#define logn_errno(id,fmt,...)log_impl_print_errno(id,__FILE__,__LINE__,3,fmt, ##__VA_ARGS__ )
#define logn_bin(id,add_msg,d,dlen)log_impl_binary(id,__FILE__,__LINE__,add_msg,d,dlen)

#define log_debug(fmt,...)logn_debug(0,fmt,##__VA_ARGS__)
#define log_info(fmt,...)logn_info(0,fmt,##__VA_ARGS__)
#define log_warn(fmt,...)logn_warn(0,fmt,##__VA_ARGS__)
#define log_error(fmt,...)logn_error(0,fmt,##__VA_ARGS__)
#define log_errno(fmt,...)logn_errno(0,fmt,##__VA_ARGS__)
#define log_bin(add_msg,d,dlen)logn_bin(0,add_msg,d,dlen)

#define std_debug(fmt,...)logn_debug(-1,fmt,##__VA_ARGS__)
#define std_info(fmt,...)logn_info(-1,fmt,##__VA_ARGS__)
#define std_warn(fmt,...)logn_warn(-1,fmt,##__VA_ARGS__)
#define std_error(fmt,...)logn_error(-1,fmt,##__VA_ARGS__)
#define std_errno(fmt,...)logn_errno(-1,fmt,##__VA_ARGS__)
#define std_bin(add_msg,d,dlen)logn_bin(-1,add_msg,d,dlen)

#endif


