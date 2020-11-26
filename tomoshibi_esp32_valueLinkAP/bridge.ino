#include "header.h"
#include "valTalkerBuf.h"

WebSocketsServer webSocket(81);

void broadcastJsonStr(const char *str){
  webSocket.broadcastTXT(str,strlen(str));
}
void broadcastJson(const MyJsonDocument &doc){
  size_t jsonTextLength = measureJson(doc) + 1;
  char jsonText[jsonTextLength]="";
  serializeJson(doc, jsonText, jsonTextLength);
  Serial.printf("broadcast json : %s\n",jsonText);
  webSocket.broadcastTXT(jsonText,jsonTextLength-1);
}

void sendJson(uint8_t num,const MyJsonDocument &doc){
  size_t jsonTextLength = measureJson(doc) + 1;
  char jsonText[jsonTextLength]="";
  serializeJson(doc, jsonText, jsonTextLength);
  Serial.printf("send to %d json : %s\n",num,jsonText);
  webSocket.sendTXT(num,jsonText,jsonTextLength-1);
}

void printJson(const MyJsonDocument &doc){
  size_t jsonTextLength = measureJson(doc) + 1;
  char jsonText[jsonTextLength]="";
  serializeJson(doc, jsonText, jsonTextLength);
  Serial.printf("printJson : %s\n",jsonText);
}

class valTalkListener{
  private:
  //valList list;
  MyJsonDocument doc;
  valTalkerBuf buf;

  void mergeJsonDoc(MyJsonDocument &sourceDoc,MyJsonDocument &targetDoc){
    JsonObject source=sourceDoc.as<JsonObject>();
    JsonObject target=targetDoc.as<JsonObject>();
    for(JsonPair i:source){
      target[i.key()]=i.value();
    }
  }
  void parseBufTo(MyJsonDocument &tmp){
    deserializeJson(tmp,(const char*)buf.getBufP(),strlen((const char*)buf.getBufP()));
    Serial.printf("size %d parse ",strlen((const char*)buf.getBufP()));
    Serial.println((const char*)buf.getBufP());
  }
  
  public:
  valTalkListener(valTalkerArduinoSerialInterface &srl):
    buf(&srl)
  {
    JsonObject obj = doc.to<JsonObject>();
  }
  void sendFullJsonTo(uint8_t num){
    sendJson(num,doc);
  }
  void sendStrToMachine(uint8_t *str){
    buf.send(str,(uint16_t)strlen((const char*)str)+1);
  }
  void loop(){
    buf.check();
    if(buf.available()){
      Serial.println("buf available!");
      broadcastJsonStr((const char*)buf.getBufP());
      MyJsonDocument tmp;
      parseBufTo(tmp);
      mergeJsonDoc(tmp,doc);
      printJson(doc);
      buf.beUnavailable();
    }
  }
};
valTalkerArduinoHardwareSerial serialRapper(Serial1);
valTalkListener listener(serialRapper);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  Serial.print("called ws event:");
  switch(type) {
    case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
    
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      //webSocket.sendTXT(num, "Connected");
      listener.sendFullJsonTo(num);
    }
    break;
    
    case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);
    listener.sendStrToMachine(payload);
    // send message to client
    // webSocket.sendTXT(num, "message here");

    // send data to all connected clients
    // webSocket.broadcastTXT("message here");
    break;
    
    case WStype_BIN:
    Serial.printf("[%u] get binary length: %u\n", num, length);
    //hexdump(payload, length);

    // send message to client
    // webSocket.sendBIN(num, payload, length);
    break;
    default:
    break;
  }

}

void setupBridge(){
  //serialRapper.begin(115200);
  Serial1.begin(115200,SERIAL_8N1,26,27);

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loopBridge(){
  listener.loop();
  webSocket.loop();
}
