#ifndef AJUST_FLOAT_VL_HPP_INCLUDE_GUARD
#define AJUST_FLOAT_VL_HPP_INCLUDE_GUARD

#include "mbed.h"
#include "useful.hpp"
#include "baseVL.hpp"

namespace vl{

/*class valFloat:public val{
  private:
  union lookFloat{
    float v;
    struct B{
      uint8_t a;
      uint8_t b;
      uint8_t c;
      uint8_t d;
    };
    B b;
  };
  enum{
    BYTE_A,BYTE_B,BYTE_C,BYTE_D
  };
  float value;
  public:
  valFloat(const char *nameArg):
    val(nameArg,"valFloat"){}
  void writeJsonValTo(JsonVariant &vala)override{
    vala.set(value);
  }
  void setVal(const float vala){
    value=vala;
  }
  void setVal(uint8_t* array,uint8_t* limit){
    lookFloat v;
    v.b.a=array[BYTE_A];
    v.b.b=array[BYTE_B];
    v.b.c=array[BYTE_C];
    v.b.d=array[BYTE_D];
    setVal(v.v);
  }
};*/
class sameFloat{
	private:
	union lookFloat{
		float v;
		uint32_t i;
	};
	lookFloat pre;
	public:
	sameFloat(){pre.v=68748.1164687;}//被らなそうな任意の値
	bool isEqu(const float val){
		lookFloat now;
		now.v=val;
		const bool ret=now.i==pre.i;
		pre=now;
		return ret;
	}
	bool operator()(const float val){
		return isEqu(val);
	}
};



class ajustVLbaseBase{
	protected:
	static const int NAME_LEN=32;
	char name[NAME_LEN];
	char smallID;
	char smallIDToBigID(const char c){return c-'a'+'A';}
	
	public:
	ajustVLbaseBase(const char *);
	virtual void publishNameTo(addValAndStrInterface*);
	virtual void publishValTo(addValAndStrInterface*)=0;
	virtual void publishStdTo(addValAndStrInterface*)=0;
	void publishAllTo(addValAndStrInterface*);
	
	virtual void ajust(const float)=0;
};
class ajustVLmanager{
	private:
	static const int LIST_LEN=26;
	ajustVLbaseBase *list[LIST_LEN];
	int listTailIndex;
	simpleJsonGenerator jsonGen;
	char indexToSmallID(int i){return i+'a';}
	int smallIDToIndex(char c){return c-'a';}
	public:
	ajustVLmanager();
	char entryAndGetSmallID(ajustVLbaseBase*);
	
	char* getAllJsonStr();
	char* getStdJsonStr();
	
	ajustVLbaseBase *refBySmallID(const char);
};
class ajustVLbase:
	public ajustVLbaseBase
{
	public:
	ajustVLbase(ajustVLmanager&,const char*);
};


class ajustFloatVL:
	public ajustVLbase
{
	private:
	float *val;
	const float step;
	sameFloat isSame;
	
	public:
	ajustFloatVL(ajustVLmanager&,const char*,float*,const float);
	void publishValTo(addValAndStrInterface*);
	void publishStdTo(addValAndStrInterface*);
	
	void ajust(const float);
};

}

#endif