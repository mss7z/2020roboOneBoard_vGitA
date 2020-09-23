#ifndef AXBEE_CORE_HPP_INCLUDE_GUARD
#define AXBEE_CORE_HPP_INCLUDE_GUARD

#include "mbed.h"
namespace rob{

#define AXBEE_RBUFF_SIZE 255
#define AXBEE_IF_RECEIVE_CALLBACK_MAXSIZE 16

#define AXBEE_16BIT_ADDR_LENGTH 2
#define AXBEE_64BIT_ADDR_LENGTH 8

struct xbeeArrayNode{
	const uint8_t *p;
	const uint16_t length;
};

namespace aXbeeCore_internal{
	
	
	class aXbeeCore{
	protected:
		Serial srl;
		void ifReceive();
		int rcounter;
	//	static const int rbuffSize;
		uint8_t rbuff[AXBEE_RBUFF_SIZE];
		unsigned int rbuffTotal;
		
		uint16_t rframeSize;
		int rframeCounter;
		//void (*ifReceiveFrame)(uint8_t*,uint16_t);
		Callback<void(uint8_t*,uint16_t)> ifReceiveFrame;
	public:
		aXbeeCore(PinName tx,PinName rx,int baudrate=9600);
		
		void sendFrame(const uint8_t frameData[],const uint16_t frameDataSize);
		void sendFrame(const xbeeArrayNode* frameData,const uint16_t nodeSize);
		
		virtual void callbackFrame(Callback<void(uint8_t*,uint16_t)>)=0;
		
		void baud(int baudrate){srl.baud(baudrate);}
	};
	
	class aXbeeCoreSingleCallback:
		public aXbeeCore
	{
		public:
		aXbeeCoreSingleCallback(PinName tx,PinName rx,int baudrate=9600):
			aXbeeCore(tx,rx,baudrate){}
		void callbackFrame(void (*fp)(uint8_t*,uint16_t));
		void callbackFrame(Callback<void(uint8_t*,uint16_t)>);
	};

	class aXbeeCoreMultiCallback:
		public aXbeeCore
	{
		private:
		int ifReceiveCallbackAlwaysCont;
		Callback<void(uint8_t*,uint16_t)> ifReceiveCallbackAlways[AXBEE_IF_RECEIVE_CALLBACK_MAXSIZE];
		void callbackFrameRepresentative(uint8_t*,uint16_t);
		
		public:
		aXbeeCoreMultiCallback(PinName tx,PinName rx,int baudrate=9600);
		int addCallbackFrame(Callback<void(uint8_t*,uint16_t)>);
		void callbackFrame(Callback<void(uint8_t*,uint16_t)> cb){addCallbackFrame(cb);}
		
		void delCallbackFrame(){ifReceiveCallbackAlwaysCont=0;}
	};
}
using aXbeeCore_internal::aXbeeCore;
using aXbeeCore_internal::aXbeeCoreSingleCallback;
using aXbeeCore_internal::aXbeeCoreMultiCallback;

}

#endif