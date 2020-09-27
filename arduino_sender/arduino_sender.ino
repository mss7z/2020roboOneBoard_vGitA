
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
  rob::aXbeeCoreCallback<1> xbeeCore(&xbeeSerial);
  rob::aXbeeCom xbee(xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x3B));

  void setupCom();
  void loopCom();
  byte genButtonBit(const int val, const byte shift);
  void sendControll();

  void setupCom(){
    xbeeSerial.begin(38400);
  }
  void loopCom(){
    static regularC sendInterval(100);
    //Serial.print("hey");
    if(sendInterval){
      sendControll();
    }
  }

  byte genButtonBit(const int val,const byte shift){
    return ((byte)(val!=0))<<shift;
  }
  void sendControll(){
    PSX.updateState(PS);
    byte buttonBit=0;
    enum{
      FORWARD_BTN,
      REVERSE_BTN,
      DEG_UP_BTN,
      DEG_DOWN_BTN,
      DEG_ZERO_BTN,
      KILL_BTN,
    };
    buttonBit|=genButtonBit(IS_DOWN_UP(PS),FORWARD_BTN);
    buttonBit|=genButtonBit(IS_DOWN_DOWN(PS),REVERSE_BTN);
    buttonBit|=genButtonBit(IS_DOWN_TRIANGLE(PS),DEG_UP_BTN);
    buttonBit|=genButtonBit(IS_DOWN_CROSS(PS),DEG_DOWN_BTN);
    buttonBit|=genButtonBit(IS_DOWN_CIRCLE(PS),DEG_ZERO_BTN);
    buttonBit|=genButtonBit(PRESSED_SQUARE(PS),KILL_BTN);
    byte sendArray[]={
      ANALOG_LEFT_Y(PS),
      ANALOG_RIGHT_Y(PS),
      buttonBit,
    };
    xbee.send(sendArray,ARRAYLEN(sendArray));
  }
}
void setup() {
  //dcom
  dcom.begin(9600);
  PSX.mode(PS,MODE_ANALOG,MODE_LOCK);
  //Serial.begin(38400);
  //Serial.print("hello");
  com::setupCom();
}

void loop() {
  com::loopCom();
}
