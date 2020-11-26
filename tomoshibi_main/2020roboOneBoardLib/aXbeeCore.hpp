#ifndef AXBEE_CORE_HPP_INCLUDE_GUARD
#define AXBEE_CORE_HPP_INCLUDE_GUARD

#include "mbed.h"
#include "useful.hpp"
namespace rob{

const int AXBEE_16BIT_ADDR_LENGTH=2;
const int AXBEE_64BIT_ADDR_LENGTH=8;

struct xbeeArrayNode{
	const uint8_t *p;
	const uint16_t length;
};

class aXbeeCallbackInterface{
	public:
	virtual void callback(uint8_t[],uint16_t)=0;
};


class aXbeeCoreInterface{
	public:
	virtual void sendFrame(const uint8_t frameData[],const uint16_t frameDataSize)=0;
	virtual void sendFrame(const xbeeArrayNode* frameData,const uint16_t nodeSize)=0;
	virtual void setCallback(aXbeeCallbackInterface*)=0;
};
class aXbeeCoreBase:
	public aXbeeCoreInterface
{
protected:
	Serial srl;
	void ifReceive();
	int rcounter;
	static const int AXBEE_RBUFF_SIZE=255;
	uint8_t rbuff[AXBEE_RBUFF_SIZE];
	unsigned int rbuffTotal;
	
	uint16_t rframeSize;
	int rframeCounter;
	//void (*ifReceiveFrame)(uint8_t*,uint16_t);
	//Callback<void(uint8_t*,uint16_t)> ifReceiveFrame;
	
	virtual void ifReceiveFrame(uint8_t[],uint16_t)=0;
public:
	aXbeeCoreBase(PinName tx,PinName rx,int baud);
	
	void sendFrame(const uint8_t frameData[],const uint16_t frameDataSize);
	void sendFrame(const xbeeArrayNode* frameData,const uint16_t nodeSize);
};


template<int N>
class aXbeeCoreCallback:
	public aXbeeCoreBase
{
	private:
	int callbackArrayCont;
	aXbeeCallbackInterface *callbackArray[N];
	public:
	aXbeeCoreCallback(PinName tx,PinName rx,int baud=9600);
	int addCallback(aXbeeCallbackInterface*);
	void setCallback(aXbeeCallbackInterface *p){addCallback(p);}
	protected:
	void ifReceiveFrame(uint8_t[],uint16_t);
};


template<int N>
inline aXbeeCoreCallback<N>::aXbeeCoreCallback(PinName tx,PinName rx,int baud):
	aXbeeCoreBase(tx,rx,baud),callbackArrayCont(0)
{
	for(int i=0;i<N;i++){
		callbackArray[i]=NULL;
	}
}
template<int N>
inline int aXbeeCoreCallback<N>::addCallback(aXbeeCallbackInterface *p){
	if(!(callbackArrayCont<N))return -1;
	callbackArray[callbackArrayCont]=p;
	return callbackArrayCont++;
}
template<int N>
inline void aXbeeCoreCallback<N>::ifReceiveFrame(uint8_t array[],uint16_t arrayLen){
	for(int i=0;i<callbackArrayCont;i++){
		callbackArray[i]->callback(array,arrayLen);
	}
}


template<>
class aXbeeCoreCallback<1>:
	public aXbeeCoreBase
{
	private:
	aXbeeCallbackInterface *callbackClass;
	public:
	aXbeeCoreCallback(PinName tx,PinName rx,int baud=9600);
	int addCallback(aXbeeCallbackInterface*);
	void setCallback(aXbeeCallbackInterface *p){addCallback(p);}
	protected:
	void ifReceiveFrame(uint8_t[],uint16_t);
};
inline aXbeeCoreCallback<1>::aXbeeCoreCallback(PinName tx,PinName rx,int baud):
	aXbeeCoreBase(tx,rx,baud)
{}
inline int aXbeeCoreCallback<1>::addCallback(aXbeeCallbackInterface *p){
	if(callbackClass!=NULL)return -1;
	callbackClass=p;
	return 1;
}
inline void aXbeeCoreCallback<1>::ifReceiveFrame(uint8_t array[],uint16_t arrayLen){
	if(callbackClass==NULL)return;
	callbackClass->callback(array,arrayLen);
}

}

#endif