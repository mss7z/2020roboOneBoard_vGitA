#ifndef AXBEE_H_INCLUDE_GUARD
#define AXBEE_H_INCLUDE_GUARD

#include "aXbeeCore.h"

namespace rob{
  
struct xbee64bitAddress{
  uint8_t address[AXBEE_64BIT_ADDR_LENGTH];
  xbee64bitAddress(const uint8_t a,const uint8_t b,const uint8_t c,const uint8_t d,const uint8_t e,const uint8_t f,const uint8_t g,const uint8_t h){
    address[0]=a;address[1]=b;address[2]=c;address[3]=d;address[4]=e;address[5]=f;address[6]=g;address[7]=h;
  }
  //bool operator== const(const xbee64bitAddress &);
};

class aXbeeCom{
  private:
  aXbeeCore &xbeeParent;
  Callback<void(uint8_t*,uint16_t)> ifReceive;
  const xbee64bitAddress pairAddr;
  uint8_t pairAddr16[AXBEE_16BIT_ADDR_LENGTH];
  void ifReceiveFrame(uint8_t*,uint16_t);
  
  public:
  aXbeeCom(aXbeeCore&,const xbee64bitAddress&);
  void attach(Callback<void(uint8_t*,uint16_t)>);
  void send(uint8_t*,uint16_t);
};

}

#endif
