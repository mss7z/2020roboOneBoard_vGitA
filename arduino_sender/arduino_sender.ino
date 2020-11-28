

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

  bool isControllerReadable=false;
  bool isReadTime(){return isControllerReadable;}
}
namespace com{
  const size_t SEND_ARRAY_LEN=4;
  
  rob::aXbeeArduinoHardwareSerial xbeeSerial(Serial);
  rob::aXbeeCoreCallback<2> xbeeCore(&xbeeSerial);
  rob::aXbeeCom tomoshibi(xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x3B));
  rob::aXbeeCom dragon(xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x4D));
  rob::aXbeeCom *xbeeP=&tomoshibi;
  
  void setupCom();
  void loopCom();
  byte genButtonBit(const int val, const byte shift);
  bool isSameArray(const uint8_t *a,const uint8_t *b,const uint16_t len);
  void sendControllNoSame(uint8_t*);
  void sendControll();
  void sendEmptyControll();
  void printLcd(uint8_t[],uint16_t);

  void setupCom(){
    xbeeSerial.begin(115200);
    tomoshibi.attach(printLcd);
    //dragon.attach(printLcd);
  }
  void loopCom(){
    //static regularC sendInterval(100);
    //Serial.print("hey");
    //if(sendInterval){
    if(base::isReadTime()){
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
  bool isSameArray(const uint8_t *a,const uint8_t *b,const uint16_t len){
    for(uint16_t i=0;i<len;i++){
      if(a[i]!=b[i]){
        return false;
      }
    }
    return true;
  }
  void sendControllNoSame(uint8_t *array){
    static uint8_t preArray[SEND_ARRAY_LEN]={};
    static timesC forceSendTime(30);
    if(isSameArray(array,preArray,SEND_ARRAY_LEN) && !forceSendTime){
      return;
    }
    xbeeP->send(array,SEND_ARRAY_LEN);
    for(uint16_t i=0;i<SEND_ARRAY_LEN;i++){
      preArray[i]=array[i];
    }
  }
  void sendControll(){
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
    byte sendArray[SEND_ARRAY_LEN]={
      ANALOG_RIGHT_Y(PS),
      ANALOG_RIGHT_X(PS),
      buttonBit1,
      buttonBit2,
    };
    //xbeeP->send(sendArray,ARRAYLEN(sendArray));
    sendControllNoSame(sendArray);
  }
  void sendEmptyControll(){
    byte sendArray[SEND_ARRAY_LEN]={
      128,
      128,
      0,
      0,
    };
    //xbeeP->send(sendArray,ARRAYLEN(sendArray));
    sendControllNoSame(sendArray);
  }
  void printLcd(uint8_t array[],uint16_t arrayLen){
    if(arrayLen<2){
      return;
    }
    lcd.setCursor(array[0],array[1]);
    lcd.print((char*)(array+2));
  }
  void printConnectedMachine(const char c){
    lcd.setCursor(13,0);
    lcd.print(c);
  }
    
  void turnMachine(){
    lcd.clear();
    xbeeP->attach(NULL);
    if(xbeeP==&tomoshibi){
      xbeeP=&dragon;
      printConnectedMachine('D');
    }else{
      xbeeP=&tomoshibi;
      printConnectedMachine('t');
    }
    xbeeP->attach(printLcd);
  }
}

void setupPS2(){
  PSX.mode(PS,MODE_ANALOG,MODE_LOCK);
}

void checkPS2(){
  //static regularC checkTime(1440);
  static const int ANALOG_ERR=255;
  if(base::isReadTime()){
    if(ANALOG_RIGHT_Y(PS)==ANALOG_ERR && ANALOG_RIGHT_X(PS)==ANALOG_ERR){
      setupPS2();
    }
    if(PRESSED_R1(PS)){
      com::turnMachine();
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
  static regularC readTime(100);
  if(readTime){
    PSX.updateState(PS);
    base::isControllerReadable=true;
  }
  com::loopCom();
  checkPS2();
  //lockPS2();
  base::isControllerReadable=false;
}
