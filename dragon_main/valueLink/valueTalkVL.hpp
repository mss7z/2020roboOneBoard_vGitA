#ifndef VALUE_LINK_HPP_INCLUDE_GUARD
#define VALUE_LINK_HPP_INCLUDE_GUARD

#include "mbed.h"
#include "useful.hpp"

namespace vl{

class valTalkerArduinoSerialInterface{
  public:
  virtual int read(void)=0;
  virtual size_t write(uint8_t c)=0;
  virtual int available(void)=0;
};

class valTalkerMbedSerial:
  public valTalkerArduinoSerialInterface
{
  protected:
  static const uint32_t BUF_MASK=0x7FF;
  uint8_t buf[BUF_MASK+1];
  uint32_t head,tail;
  Serial &srl;
  void callbackFunc();
  public:
  valTalkerMbedSerial(Serial &srlArg):
	head(0),tail(0),srl(srlArg){
	  srl.attach(callback(this,&valTalkerMbedSerial::callbackFunc),Serial::RxIrq);
  }
  int read(void){
	if(!available())return -1;
	return buf[(head++)&BUF_MASK];
  }
  size_t write(uint8_t c){return srl.putc(c);}
  //不完全な互換性
  int available(void){ return (int)((head&BUF_MASK)!=(tail&BUF_MASK));}
};


class valTalkerBuf{
  private:
  valTalkerArduinoSerialInterface *srl;
  static const uint16_t BUF_LEN=1023;
  uint8_t buf[BUF_LEN];
  uint8_t *bufTailP,*bufTailLimitP;
  uint16_t receiveLen;
  uint32_t receiveSum;
  bool isAvailable;
  uint16_t status;
  
  enum{
    STATUS_BEGIN,STATUS_LENA,STATUS_LENB,STATUS_BUF,STATUS_CHK
  };
  
  void readByte(){
    const uint8_t c=srl->read()&0xFF;
    switch(status){
      case STATUS_BEGIN:
      if(c==0xF8){
        status++;
      }
      break;

      case STATUS_LENA:
      receiveLen=c<<8;
      status++;
      break;

      case STATUS_LENB:
      receiveLen|=c;
      if((receiveLen<BUF_LEN)){
        bufTailP=buf;
        bufTailLimitP=buf+receiveLen;
        receiveSum=0;
        status++;
      }else{
        status=STATUS_BEGIN;
      }
      break;

      case STATUS_BUF:
      *bufTailP=c;
      receiveSum+=c;
      bufTailP++;
      if(!(bufTailP<bufTailLimitP)){
        status++;
      }
      break;

      case STATUS_CHK:
      if(c==(receiveSum&0xFF)){
		isAvailable=true;
      }
	  status=STATUS_BEGIN;
      break;
    }
  }
  
  public:
  valTalkerBuf(valTalkerArduinoSerialInterface *srla):
    srl(srla),isAvailable(false),status(0){}
  
  void send(uint8_t* array, uint16_t len){
    uint32_t sum=0;
    srl->write(0xF8);
    srl->write((len>>8)&0xFF);
    srl->write((len)&0xFF);
    for(uint16_t i=0;i<len;i++){
      srl->write(array[i]);
      sum+=array[i];
    }
    srl->write(sum&0xFF);
  }
  
  void check(){
    if(srl->available() && !isAvailable){
      readByte();
    }
  }
  uint8_t* getBufP(){
    return buf;
  }
  uint8_t* getBufLimitP(){
    return bufTailLimitP;
  }
  uint16_t getLen(){
    return receiveLen;
  }
  bool available(){
	  return isAvailable;
  }
  void beUnavailable(){
    isAvailable=false;
  }
};


}

#endif