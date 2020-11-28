#ifndef BASE_VL_HPP_INCLUDE_GUARD
#define BASE_VL_HPP_INCLUDE_GUARD

#include "mbed.h"

namespace vl{
class addValAndStrInterface{
	public:
	virtual void addVal(const char*,const float)=0;
	virtual void addStr(const char*,const char*)=0;
	virtual void addVal(const char,const float)=0;
	virtual void addStr(const char,const char*)=0;
};
class simpleJsonGenerator:
	public addValAndStrInterface
{
	private:
	static const int BUF_LEN=1024;
	char buf[BUF_LEN];
	char *tail;
	
	public:
	void initialize();
	void addVal(const char*,const float);
	void addStr(const char*,const char*);
	void addVal(const char,const float);
	void addStr(const char,const char*);
	void finalize();
	
	char* getStr(){return buf;}
};
}

#endif