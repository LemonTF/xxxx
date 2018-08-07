#ifndef __HASH_HPP__
#define __HASH_HPP__
#include <stdint.h>
#include <string>

uint32_t 	hash(const void*d,int len);
std::string format_bin(const char*d,int len);
std::string format_bin2(const char*d,int len);

#endif
