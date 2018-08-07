#include <stdint.h>
#include <algorithm>
#include <array>
#include <string.h>
#include <tools.h>

uint32_t hash(const void*d,int len)
{
	uint8_t*db=(uint8_t*)d;
	uint32_t h=0;
	for(int i=0;i<len;i++)
	{
		h*=50793233;
		h^=db[i];
	}
	h ^= (h>>13) ^ (h>>23);
	return h ^ (h >> 9) ^ (h >> 5);
}

struct bin_format
{
	char *_buff;
	char *code1[256],  *code2[256];

	bin_format()
	{
		_buff=(char*)malloc(1024+512);

		int n;
		char*p=_buff;
		for(int i=0;i<256;i++)
		{
			n=sprintf(p,"%02X ",i); 
			code1[i]=p;
			p+=n+1;

			n=sprintf(p,"%c",std::isprint(i)?i:'.'); 
			code2[i]=p;
			p+=n+1;
		}
	}

	~bin_format()
	{
		free(_buff);
	}

	void format_line2(std::string&rt,const char*b,const char*e)
	{
		char b1[256];
		char*p1=b1;
		
		for(;b!=e;b++)
		{
			uint8_t ch=*b;
			strcpy(p1,code1[ch]); p1+=3;
		}
		*p1=0;

		rt.append(b1,p1);
		rt.append("\n");
	}

	std::string do_format2(const char*d,size_t len)
	{
		std::string rt;
		rt.reserve(len*4);
		int line=len>>5;
		for(int i=0;i<line;i++)
		{
			const char*t=d+(i<<5);
			format_line2(rt,t,t+32);
		}

		if(len%32)
		{
			format_line2(rt,d+(len>>5<<5),d+len);
		}

		return std::move(rt);
	}

	void format_line(std::string&rt,const char*b,const char*e)
	{
		char b1[256], b2[128];
		char*p1=b1,*p2=b2;
		
		for(;b!=e;b++)
		{
			uint8_t ch=*b;
			strcpy(p1,code1[ch]); p1+=3;
			strcpy(p2,code2[ch]); p2+=1;
		}
		*p1=0;*p2=0;

		rt.append(b1,p1);
		rt.append(48-(p1-b1)+8,' ');
		rt.append(b2,p2);
		rt.append("\n");
	}

	std::string do_format(const char*d,size_t len)
	{
		std::string rt;
		rt.reserve(len*4+len);
		int line=len>>4;
		for(int i=0;i<line;i++)
		{
			const char*t=d+(i<<4);
			format_line(rt,t,t+16);
		}

		if(len%16)
		{
			format_line(rt,d+(len>>4<<4),d+len);
		}

		return std::move(rt);
	}
};

std::string format_bin(const char*d,int len)
{
	static bin_format format;
	return std::move(format.do_format(d,len));
}

std::string format_bin2(const char*d,int len)
{
	static bin_format format;
	return std::move(format.do_format2(d,len));
}

