#define ARRAYSIZE(X) (sizeof(X)/sizeof(X[0]))

#include "2020roboOneBoardLibSender/lib.h"

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

rob::aXbeeCoreMultiCallback<HardwareSerial> xbeeCore(&Serial);
rob::aXbeeCom xbee(xbeeCore,rob::xbee64bitAddress(0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35));

void setup() {
  //dcom
  dcom.begin(9600);
}

void loop() {
}
