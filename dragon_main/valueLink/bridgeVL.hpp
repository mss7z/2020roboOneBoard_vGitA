#ifndef BRIDGE_VL_HPP_INCLUDE_GUARD
#define BRIDGE_VL_HPP_INCLUDE_GUARD

#include "mbed.h"
#include "valueTalkVL.hpp"
#include "ajustVL.hpp"
#include "useful.hpp"
#include "buttonVL.hpp"


namespace vl{

class regularC_ms{
private:
	unsigned long interval;
	unsigned long nextTime;
	Timer t;
public:
	regularC_ms(unsigned long intervalArg,unsigned long start=0):
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

class timesC{
	private:
	int cont;
	const int times;
	public:
	timesC(const int timesa):cont(0),times(timesa){}
	bool is(){
		if(cont>=times){
			cont=0;
			return true;
		}
		cont++;
		return false;
	}
	operator bool(){return is();}
};
		
class valueLinkCore{
	private:
	Serial &rawSrl;
	valTalkerMbedSerial srl;
	valTalkerBuf valTalker;
	
	ajustVLmanager vlManager;
	crossBtnVL crossBtn;
	
	static const int sendInterval=100;//ms
	regularC_ms sendTime;
	static const int forceSendInterval=20;//times
	timesC forceSendTime;
	
	bool isEmptyJsonStr(const char*);
	
	void sendStr(const char *);
	void send();
	void receive();
	
	public:
	valueLinkCore(Serial&);
	void setup();
	void loop();
	
	ajustVLmanager& refManager(){
		return vlManager;
	}
	operator ajustVLmanager& (){
		return refManager();
	}
	
	void attachBtn(Callback<void(char)> func){
		crossBtn.attach(func);
	}
	
};

}
	
#endif