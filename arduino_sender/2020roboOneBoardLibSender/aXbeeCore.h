#ifndef AXBEE_CORE_H_INCLUDE_GUARD
#define AXBEE_CORE_H_INCLUDE_GUARD

#include "Callback.h"

namespace rob{
using namespace mbed;

#ifndef ARRAYLEN
#define ARRAYLEN(X) (sizeof(X)/sizeof(X[0]))
#endif

#define AXBEE_IF_RECEIVE_CALLBACK_MAXSIZE 16

#define AXBEE_16BIT_ADDR_LENGTH 2
#define AXBEE_64BIT_ADDR_LENGTH 8

struct xbeeArrayNode{
  const uint8_t *p;
  const uint16_t length;
};

class aXbeeCore{
  public:
  virtual void check()=0;
  virtual void sendFrame(const byte frameData[],const byte frameDataSize)=0;
  virtual void sendFrame(const xbeeArrayNode node[],const byte nodeSize)=0;
  virtual void callbackFrame(Callback<void(uint8_t*,uint16_t)>)=0;
};

template<class SRL>
class aXbeeCoreBase:
  public aXbeeCore
{
protected:
  static const int AXBEE_RBUFF_SIZE=255;
  static const int AXBEE_TIMEOUT=3000;//us
  SRL *srl;
  void ifReceive();
  
  byte rcounter;
  uint8_t rbuff[AXBEE_RBUFF_SIZE];
  unsigned int rbuffTotal;
  
  byte rframeSize;
  int rframeCounter;
  //void (*ifReceiveFrame)(uint8_t*,uint16_t);
  Callback<void(uint8_t*,uint16_t)> ifReceiveFrame;
  
  int getByte();
public:
  aXbeeCoreBase(SRL*,const unsigned long);
  void check();
  void sendFrame(const byte frameData[],const byte frameDataSize);
  void sendFrame(const xbeeArrayNode node[],const byte nodeSize);
};

template<class SRL>
class aXbeeCoreSingleCallback:
  public aXbeeCoreBase<SRL>
{
  public:
  aXbeeCoreSingleCallback(SRL* srl,const unsigned long baud=9600):
    aXbeeCoreBase<SRL>::aXbeeCoreBase(srl,baud){}
  void callbackFrame(void (*fp)(uint8_t*,uint16_t));
  void callbackFrame(Callback<void(uint8_t*,uint16_t)>);
};

template<class SRL>
class aXbeeCoreMultiCallback:
  public aXbeeCoreBase<SRL>
{
  private:
  int ifReceiveCallbackAlwaysCont;
  Callback<void(uint8_t*,uint16_t)> ifReceiveCallbackAlways[AXBEE_IF_RECEIVE_CALLBACK_MAXSIZE];
  void callbackFrameRepresentative(uint8_t*,uint16_t);
  
  public:
  aXbeeCoreMultiCallback(SRL* srl,const unsigned long baud=9600);
  int addCallbackFrame(Callback<void(uint8_t*,uint16_t)>);
  void callbackFrame(Callback<void(uint8_t*,uint16_t)> cb){addCallbackFrame(cb);}
  
  void delCallbackFrame(){ifReceiveCallbackAlwaysCont=0;}
};

}//namespace rob
#endif
