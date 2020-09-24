
#define ARRAYLEN(X) (sizeof(X)/sizeof(X[0]))

#include "lib.h"

#include <GPSX.h>
#include <GPSXClass.h>
#define PS PSX_PAD1
#include <SoftwareSerial.h>
//                    TX RX
SoftwareSerial dcom(A4,A5,false);

class regularC{
private:
  unsigned long interval;
  unsigned long nextTime;
public:
  regularC(unsigned long intervalArg,unsigned long start=0):
    interval(intervalArg)
  {
    nextTime=start;
  }
  bool ist(){
    if(nextTime<millis()){
      nextTime=interval+millis();
      return true;
    }else{
      return false;
    }
  }
  operator bool(){return ist();}
  void set(unsigned long val){interval=val;}
  unsigned long read(){return interval;}
};

template <typename T>
class delta{
private:
  T preVal;
public:
  delta():preVal((T)0){}
  T f(T val){
    const T ans=(val-preVal);
    preVal=val;
    return ans;
  }
};


//rob::aXbeeCore<HardwareSerial> temp(&Serial);

namespace com{
  rob::aXbeeArduinoHardwareSerial xbeeSerial(Serial);
  rob::aXbeeCoreCallback<1> xbeeCore(&xbeeSerial,38400);
  rob::aXbeeCom xbee(xbeeCore,rob::xbee64bitAddress(0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35));

  void loopCom(){
    static regularC sendInterval(100);
    if(sendInterval){
      
    }
  }

  void sendControll(){
    PSX.updateState(PS);
    byte sendArray[]={
      ANALOG_RIGHT_Y(PS),
      ANALOG_LEFT_Y(PS)
    };
    xbee.send(sendArray,ARRAYLEN(sendArray));
  }
}
void setup() {
  //dcom
  dcom.begin(9600);
  PSX.mode(PS,MODE_ANALOG,MODE_LOCK);
}

void loop() {
}
