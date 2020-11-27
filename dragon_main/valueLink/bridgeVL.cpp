#include "bridgeVL.hpp"

namespace vl{

#include "jsmn.h"
void valueLinkCore::sendStr(const char *str){
	valTalker.send((uint8_t*)str,strlen(str)+1);
	//pc.printf("send %s\n",str);
}
bool valueLinkCore::isEmptyJsonStr(const char *str){
	return str[0]=='{' && str[1]=='}' && str[2]=='\0';
}
void valueLinkCore::send(){
	char *str=vlManager.getStdJsonStr();
	if(!isEmptyJsonStr(str) || forceSendTime){
		sendStr(str);
	}	
}
void valueLinkCore::receive(){
	const char *str=(const char*)valTalker.getBufP();
	//pc.printf("receive: %s\n",str);
	
	const int tokensSize=50;
	jsmn_parser p;
	jsmntok_t tokens[tokensSize];
	jsmn_init(&p);
	enum{
		TOKEN_SIZE=0,
		SMALL_ID=1,
		ADD_VALUE=2
	};
	
	if(0<jsmn_parse(&p,str,strlen(str),tokens,tokensSize)){
		int receiveTokensSize=tokens[TOKEN_SIZE].size;
		//pc.printf("%d\n",receiveTokensSize);
		for(int i=0;i<receiveTokensSize;i++){
			const int offset=i*2;
			//pc.printf("i=%d\n",i);
			if(tokens[SMALL_ID+offset].type!=JSMN_STRING){
				return;
			}
			const char smallID=str[tokens[SMALL_ID+offset].start];
			//pc.printf("i=%d\n",i);
			char buf[50]="";
			strncpy(buf,str+tokens[ADD_VALUE+offset].start,tokens[ADD_VALUE+offset].end-tokens[ADD_VALUE+offset].start);
			
			if(tokens[ADD_VALUE+offset].type==JSMN_PRIMITIVE){
				const float addValue=atof(buf);
				ajustVLbaseBase *refP=vlManager.refBySmallID(smallID);
				if(refP!=NULL){
					refP->ajust(addValue);
					send();
				}
			}else if(tokens[ADD_VALUE+offset].type==JSMN_STRING){
				if(smallID=='+'){
					crossBtn.listener(buf[0]);
					return;
				}
			}
			
			//pc.printf("parse smallID %c  buf %s\n",smallID,buf);
		}
	}
}

valueLinkCore::valueLinkCore(Serial &rawSrlA):
	rawSrl(rawSrlA),
	srl(rawSrl),
	valTalker(&srl),
	sendTime(sendInterval),
	forceSendTime(forceSendInterval)
{
}

void valueLinkCore::setup(){
	char *str=vlManager.getAllJsonStr();
	sendStr(str);
	sendStr(crossBtn.getJsonStr());
}

void valueLinkCore::loop(){
	if(sendTime){
		send();
		//pc.printf("tail:%d head:%d",srl.tail,srl.head);
	}
	valTalker.check();
	if(valTalker.available()){
		//pc.printf("available!\n");
		receive();
		valTalker.beUnavailable();
	}
}

}