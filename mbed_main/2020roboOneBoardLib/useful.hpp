#ifndef USEFUL_HPP_INCLUDE_GUARD
#define USEFUL_HPP_INCLUDE_GUARD

#include "mbed.h"

namespace rob{

#define ARRAYLEN(X) (sizeof((X))/sizeof((X)[0]))

namespace __flt_internal__{
	//myFlt ver.0.1 by mss7z 201908250133
	//mbedにおいて浮動小数点型をprintfで使用する
	//使い方例: pc.printf("%s\n",flt(3.141592653,5));
	char *flt(float val,const int decimal=3);
}
using __flt_internal__::flt;

namespace arduino{

	template<typename T>
	inline T map(T x, T in_min, T in_max, T out_min, T out_max){
	  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}
}

class regularC{
private:
	unsigned long interval;
	unsigned long nextTime;
	Timer t;
public:
	regularC(unsigned long intervalArg,unsigned long start=0):
	interval(intervalArg)
	{
		t.start();
		nextTime=start;
	}
	bool ist(){
		if(nextTime<(unsigned long)t.read_ms()){
			nextTime=interval+t.read_ms();
			return true;
		}else{
			return false;
		}
	}
	void set(unsigned long val){interval=val;}
	unsigned long read(){return interval;}
	operator bool(){return ist();}
};

class trueFalse{
	private:
	bool is;
	public:
	trueFalse(bool start=true):is(!start){}
	bool get(){is=!is;return is;}
	operator bool(){return get();}
};

template <typename T>
class delta{
private:
	T preVal;
	T deltaT;
public:
	delta(T dt=(T)1):preVal((T)0),deltaT(dt){}
	T f(T val){
		const T ans=(val-preVal)/deltaT;
		preVal=val;
		return ans;
	}
	void reset(){
		preVal=0;
	}
	void set(T val){
		preVal=val;
	}
};

}


#endif