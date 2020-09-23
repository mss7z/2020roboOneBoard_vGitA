#include "aXbeeCore.h"



//////////////////////////////////////////////////////////////////////////////////aXbeeCore ここから

namespace rob{

template<class SRL>
aXbeeCoreBase<SRL>::aXbeeCoreBase(SRL *srlArg,const unsigned long baud):
  rcounter(0)
{
  srl=srlArg;
  srl->begin(baud);
}

template<class SRL>
int aXbeeCoreBase<SRL>::getByte(){
  const unsigned long timeOut=micros()+AXBEE_TIMEOUT;
  while(true){
    if(srl->available()){
      return srl->read();
    }else if(micros()>timeOut){
      return -1;
    }
  }
}

//送信の本体(APIのフレームにして送信)
//フレームデータ(Frame-Specific Data)の入った配列とその長さを受けっとって送信する
//後でエスケープに対応させること(api=2)
template<class SRL>
void aXbeeCoreBase<SRL>::sendFrame(const byte frameData[],const byte frameDataSize){
  unsigned int frameDataTotal=0;
  
  //Start Delimiter
  srl->write(0x7E);

  //Length MSB
  srl->write(0x00);

  //Length LSB
  srl->write(frameDataSize);

  //Frame-Specific Data (FrameData)
  for(byte i=0;i<frameDataSize;i++){
    frameDataTotal += frameData[i];
    srl->write(frameData[i]);
  }

  //CheckSum
  srl->write(0xFF - (frameDataTotal & 0xFF));
}

template<class SRL>
void aXbeeCoreBase<SRL>::sendFrame(const xbeeArrayNode node[],const byte nodeSize){
  unsigned int frameDataTotal=0;

  //Start Delimiter
  srl->write(0x7E);
  
  uint16_t frameDataSize=0;
  for(int i=0;i<nodeSize;i++){
    frameDataSize+=node[i].length;
  }
  //Length MSB
  srl->write((frameDataSize>>8)&0xFF);

  //Length LSB
  srl->write(frameDataSize&0xFF);

  //Frame-Specific Data (FrameData)
  for(int i=0;i<nodeSize;i++){
    for(int j=0;j<node[i].length;j++){
      frameDataTotal += node[i].p[j];
      srl->write(node[i].p[j]);
    }
  }

  //CheckSum
  srl->write(0xFF - (frameDataTotal & 0xFF));

  return;
}

template<class SRL>
void aXbeeCoreBase<SRL>::check(){
  const unsigned long timeOut=micros()+AXBEE_TIMEOUT;
  while(true){
    if(srl->available()){
      ifReceive();
    }else if(micros()>timeOut){
      return;
    }
  }
}

template<class SRL>
void aXbeeCoreBase<SRL>::ifReceive(){
  const uint8_t data=(uint8_t)srl->read();
  switch(rcounter){
  case 0:
    if(data==0x7E){
      rcounter++;
    }
    break;
  case 1:
    rframeSize=(uint16_t)(data<<8);
    rcounter++;
    break;
  case 2:
    rframeSize|=(uint16_t)data;
    if(rframeSize>AXBEE_RBUFF_SIZE){
      rcounter=0;
    }else{
      rcounter++;
      rframeCounter=0;
      rbuffTotal=0;
    }
    break;
  case 3:
    rbuff[rframeCounter]=data;
    rbuffTotal+=data;
    rframeCounter++;
    if(!(rframeCounter<rframeSize)){
      rcounter++;
    }
    break;
  case 4:
    if((0xFF-(rbuffTotal&0xFF)) == data){
      if((bool)ifReceiveFrame){
        ifReceiveFrame(rbuff,rframeSize);
      }
    }
    rcounter=0;
    break;
  }
}

template class aXbeeCoreBase<HardwareSerial>;


template<class SRL>
void aXbeeCoreSingleCallback<SRL>::callbackFrame(void (*fp)(uint8_t*,uint16_t)){
  aXbeeCoreBase<SRL>::ifReceiveFrame=callback(fp);
  return;
}
template<class SRL>
void aXbeeCoreSingleCallback<SRL>::callbackFrame(Callback<void(uint8_t*,uint16_t)> cb){
  aXbeeCoreBase<SRL>::ifReceiveFrame=cb;
  return;
}
template class aXbeeCoreSingleCallback<HardwareSerial>;

template<class SRL>
int aXbeeCoreMultiCallback<SRL>::addCallbackFrame(Callback<void(uint8_t*,uint16_t)> cb){
  ifReceiveCallbackAlways[ifReceiveCallbackAlwaysCont]=cb;
  return ifReceiveCallbackAlwaysCont++;
}
template<class SRL>
void aXbeeCoreMultiCallback<SRL>::callbackFrame(Callback<void(uint8_t*,uint16_t)> cb){
  addCallbackFrame(cb);
}
  

template<class SRL>
void aXbeeCoreMultiCallback<SRL>::callbackFrameRepresentative(uint8_t *buff,uint16_t frameSize){
  for(int i=0;i<ifReceiveCallbackAlwaysCont;i++){
    ifReceiveCallbackAlways[i].call(buff,frameSize);
  }
}

template<class SRL>
aXbeeCoreMultiCallback<SRL>::aXbeeCoreMultiCallback(SRL *srl,const unsigned long baud):
  aXbeeCoreBase<SRL>::aXbeeCoreBase(srl,baud),ifReceiveCallbackAlwaysCont(0)
{
  aXbeeCoreBase<SRL>::ifReceiveFrame=callback(this,&rob::aXbeeCoreMultiCallback<SRL>::callbackFrameRepresentative);
}

template class aXbeeCoreMultiCallback<HardwareSerial>;
//template class aXbeeCoreMultiCallback<SoftwareSerial>;

}//namespace rob



//////////////////////////////////////////////////////////////////////////////////aXbeeCore ここまで
