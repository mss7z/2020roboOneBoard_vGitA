

#define ARRAYLEN(X) (sizeof(X)/sizeof(X[0]))

#include "lib.h"

#include <GPSX.h>
#include <GPSXClass.h>
#define PS PSX_PAD1
#include <SoftwareSerial.h>
//                    TX RX
SoftwareSerial dcom(A4,A5,false);
#include <LiquidCrystal.h>
LiquidCrystal lcd(8,A0,9,10,11,12);

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

namespace base{
  bool controllerLock=false;
  bool isLock(){return controllerLock;}
  bool turnLock(){return controllerLock=!controllerLock;}
}
namespace com{
  rob::aXbeeArduinoHardwareSerial xbeeSerial(Serial);
  rob::aXbeeCoreCallback<1> xbeeCore(&xbeeSerial);
  rob::aXbeeCom xbee(xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x3B));

  void setupCom();
  void loopCom();
  byte genButtonBit(const int val, const byte shift);
  void sendControll();
  void sendEmptyControll();
  void printLcd(uint8_t[],uint16_t);

  void setupCom(){
    xbeeSerial.begin(38400);
    xbee.attach(printLcd);
  }
  void loopCom(){
    static regularC sendInterval(100);
    //Serial.print("hey");
    if(sendInterval){
      if(base::isLock()){
        sendEmptyControll();
      }else{
        sendControll();
      }
    }
    xbeeCore.check();
  }

  byte genButtonBit(const int val,const byte shift){
    return ((byte)(val!=0))<<shift;
  }
  void sendControll(){
    PSX.updateState(PS);
    byte buttonBit1=0,buttonBit2=0;
    enum buttonBit1{
      UP_BTN,
      DOWN_BTN,
      RIGHT_BTN,
      LEFT_BTN,
      TRIANGLE_BTN,
      CROSS_BTN,
      CIRCLE_BTN,
      KILL_BTN,
    };
    enum buttonBit2{
      L1_BTN
    };
    buttonBit1|=genButtonBit(IS_DOWN_UP(PS),UP_BTN);
    buttonBit1|=genButtonBit(IS_DOWN_DOWN(PS),DOWN_BTN);
    buttonBit1|=genButtonBit(PRESSED_RIGHT(PS),RIGHT_BTN);
    buttonBit1|=genButtonBit(PRESSED_LEFT(PS),LEFT_BTN);
    buttonBit1|=genButtonBit(PRESSED_TRIANGLE(PS),TRIANGLE_BTN);
    buttonBit1|=genButtonBit(PRESSED_CROSS(PS),CROSS_BTN);
    buttonBit1|=genButtonBit(IS_DOWN_CIRCLE(PS),CIRCLE_BTN);
    buttonBit1|=genButtonBit(PRESSED_SQUARE(PS),KILL_BTN);
    
    buttonBit2|=genButtonBit(IS_DOWN_L1(PS),L1_BTN);
    byte sendArray[]={
      ANALOG_RIGHT_Y(PS),
      ANALOG_RIGHT_X(PS),
      buttonBit1,
      buttonBit2,
    };
    xbee.send(sendArray,ARRAYLEN(sendArray));
  }
  void sendEmptyControll(){
    byte sendArray[]={
      128,
      128,
      0,
      0,
    };
    xbee.send(sendArray,ARRAYLEN(sendArray));
  }
  void printLcd(uint8_t array[],uint16_t arrayLen){
    if(arrayLen<2){
      return;
    }
    lcd.setCursor(array[0],array[1]);
    lcd.print((char*)(array+2));
  }
}

void setupPS2(){
  PSX.mode(PS,MODE_ANALOG,MODE_LOCK);
}

void checkPS2(){
  static regularC checkTime(440);
  static const int ANALOG_ERR=255;
  if(checkTime){
    PSX.updateState(PS);
    if(ANALOG_RIGHT_Y(PS)==ANALOG_ERR && ANALOG_RIGHT_X(PS)==ANALOG_ERR){
      setupPS2();
    }
  }
}
void lockPS2(){
  static regularC lockTime(132);
  if(lockTime){
    PSX.updateState(PS);
    if(PRESSED_SELECT(PS) || PRESSED_START(PS)){
      lcd.setCursor(13,0);
      if(base::turnLock()){
        lcd.print("L");
      }else{
        lcd.print(" ");
      }
    }
  }
}
void setup() {
  //dcom
  dcom.begin(9600);
  setupPS2();
  lcd.begin(16,2);
  lcd.print("2020ROB!");
  //Serial.begin(38400);
  //Serial.print("hello");
  com::setupCom();
}

void loop() {
  com::loopCom();
  checkPS2();
  lockPS2();
}
