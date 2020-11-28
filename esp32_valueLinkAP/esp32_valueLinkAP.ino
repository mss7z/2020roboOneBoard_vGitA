/*
 * https://github.com/espressif/arduino-esp32
 * https://github.com/lorol/arduino-esp32fs-plugin
 * 上のtoolsはC:\Users\mss\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\tools
 * 
 * https://github.com/Links2004/arduinoWebSockets
 */


#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "header.h"

IPAddress appIp(192,168,0,10);
IPAddress subnet(255,255,255,0);

WebServer webApp(80);
DNSServer dnsServer;

const bool IS_DEBUG=true;
enum{
  TOMOSHIBI,
  DRAGON
};
const int TARGET_MACHINE=DRAGON;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nstarted ESP32!");

  if(IS_DEBUG){
    static const char ssid[]="Hawaii";
    static const char password[]="pass114514";

    IPAddress myIP;
    if(TARGET_MACHINE==TOMOSHIBI){
      myIP=IPAddress(10,114,51,50);
    }else if(TARGET_MACHINE==DRAGON){
      myIP=IPAddress(10,114,51,51);
    }
    IPAddress gatewayIPa(10,114,51,1);
    IPAddress subnet(255,255,255,0);
    appIp=myIP;
    
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.config(myIP,gatewayIPa,subnet);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }else{

    
    IPAddress myIp(192,168,0,10);
    appIp=myIp;
    
    Serial.print("Setting soft-AP configuration ... ");
    delay(100);
    Serial.println(WiFi.softAPConfig(appIp, appIp, subnet) ? "Ready" : "Failed!");

    delay(1000);
    
    Serial.print("Setting soft-AP ... ");
    Serial.println(WiFi.softAP("Jaxson","pass114514") ? "Ready" : "Failed!");

    delay(1000);
  
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
  }

  setupFileSystem();
  setupWebSocket();
  
  dnsServer.start(53,"jaxson.local",appIp);
  
  webApp.serveStatic("/",FSUSE,"/","no-store,max-age=0");

  webApp.onNotFound([](){
    Serial.print("URI:");
    Serial.println(webApp.uri());
    //if(!handleFromFFat(webApp.uri())){
      const char text[]="404 Not found!";
      webApp.send(404,"text/plain",text);
    //}
  });
  webApp.begin();

  setupBridge();
  
}


void loop() {
  /*static regularC dotTime(10000);
  if(dotTime){
    Serial.println("heart beat?");
  }*/
  webApp.handleClient();
  dnsServer.processNextRequest();
  loopWebSocket();
  loopBridge();
  /*static regularC webSocketSendTime(3000);
  if(webSocketSendTime){
    valFloat test("test");
    MyJsonDocument doc;
    JsonObject obj=doc.to<JsonObject>();
    test.addJsonTo(obj);
    broadcastJson(doc);
  }*/
  /*Serial1.print('h');
  if(Serial1.available()){
    Serial.print("\n");
    Serial.print(Serial1.read());
  }*/
}
