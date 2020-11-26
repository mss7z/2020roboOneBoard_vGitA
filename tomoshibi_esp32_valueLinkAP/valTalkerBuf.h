
class valTalkerArduinoSerialInterface{
  public:
  virtual void begin(unsigned long baud)=0;
  virtual int read(void)=0;
  virtual size_t write(uint8_t c)=0;
  virtual int available(void)=0;
};

template<class SRL>
class valTalkerArduinoSerial:
  public valTalkerArduinoSerialInterface
{
  protected:
  SRL &srl;
  public:
  valTalkerArduinoSerial(SRL &srlArg):srl(srlArg){}
  int read(void){return srl.read();}
  size_t write(uint8_t c){return srl.write(c);}
  int available(void){return srl.available();}
};
class valTalkerArduinoHardwareSerial:
  public valTalkerArduinoSerial<HardwareSerial>
{
  public:
  valTalkerArduinoHardwareSerial(HardwareSerial &srlArg):valTalkerArduinoSerial<HardwareSerial>(srlArg){}
  void begin(unsigned long baud){srl.begin(baud);}
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
    srl->write((len<<8)&0xFF);
    srl->write((len)&0xFF);
    for(uint16_t i=0;i<len;i++){
      srl->write(array[i]);
      Serial.printf(",%d",array[i]);
      sum+=array[i];
    }
    Serial.println();
    srl->write(sum&0xFF);
  }
  
  void check(){
    if(srl->available() && !isAvailable){
      readByte();
    }
  }
  bool available(){
    return isAvailable;
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
  void beUnavailable(){
    isAvailable=false;
  }
};
