#include "mbed.h"


#include "base.hpp"
#include "run.hpp"

#include "valueLink/lib.hpp"

namespace com{
	rob::aXbeeCom xbee(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0xF1));
	
#ifdef TARGET_IS_TOMOSHIBI
	//rob::aXbeeCom &kanto=miniKanto;
	rob::aXbeeCom kanto(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0x79));
	//以下がDragon
	rob::aXbeeCom otherMachine(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x4D));
#endif

#ifdef TARGET_IS_DRAGON
	//rob::aXbeeCom &kanto=bigKanto;
	rob::aXbeeCom kanto(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9D,0x0B));
	//rob::aXbeeCom kanto(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0xF5));
	//以下がTOMOSHIBI
	rob::aXbeeCom otherMachine(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xa2,0x00,0x40,0xCA,0x9D,0x3B));
#endif
	
	uint8_t receiveArray[255]={0};
	uint16_t receiveSize=0;
	
	const uint8_t MAX_LCD_STRLEN=100;
	
	char kantoStatus='S';
	
	//内部
	void ifReceiveFromController(uint8_t*,uint16_t);
	void ifReceiveFromOtherMachine(uint8_t*,uint16_t);
	float byte2floatMotorOutput(uint8_t);
	bool genBoolFromButtonBit(uint8_t,uint8_t);
	uint8_t calcStrLen(const char []);
	void sendToKantoByStatus();
	void sendToKantoByChar(char);
	
	//外部
	void setupCom();
	void printLcd(const uint8_t col,const uint8_t row, const char str[]);
	void setKantoStatusAndTellOtherMachine(char);
	
	class ajustFloat{
		private:
		static const int nameLen=16;
		float *valP;
		const float step;
		char name[nameLen];
		
		public:
		ajustFloat(const char n[],float *p,float st):valP(p),step(st){
			for(int i=0;i<nameLen;i++){name[i]=n[i];if(n[i]=='\0'){break;}}
			pc.printf("%s\n",rob::flt(0.0001,4));
		}
		void up(float mult){(*valP)+=step*mult;}
		void down(float mult){*valP-=step*mult;}
		void print(){printLcd(1,1,rob::flt(*valP,8));}
		void printAll(){printLcd(0,1,name);printLcd(1,1,"            ");print();}
	};
	ajustFloat ajustFloatArray[]={
		ajustFloat("t",&run::targetDeg,0.05),
		ajustFloat("P",&run::degGainP,0.0001),
		ajustFloat("I",&run::degGainI,0.0001),
		ajustFloat("D",&run::degGainD,0.0000001),
		ajustFloat("p",&run::targetDegGainP,0.000001),
		ajustFloat("i",&run::targetDegGainI,0.0000001),
		ajustFloat("d",&run::targetDegGainD,0.00000001),
		ajustFloat("m",&run::targetDegBaseChangeMult,0.001),
	};
	class ajustFloatManager{
		private:
		ajustFloat *array;
		const int end;
		static const int start=0;
		int now;
		public:
		ajustFloatManager(ajustFloat *ar,const int len):array(ar),end(len-1),now(start){}
		void printAll(){array[now].printAll();}
		void next(){if(now==end){now=start;}else{now++;}printAll();}
		void back(){if(now==start){now=end;}else{now--;}printAll();}
		void print(){array[now].print();}
		void up(float mult=1.0){array[now].up(mult);}
		void down(float mult=1.0){array[now].down(mult);}
	};
	ajustFloatManager ajust(ajustFloatArray,ARRAYLEN(ajustFloatArray));
	
	void ifReceiveFromController(uint8_t *array,uint16_t size){
		for(int i=0;i<size;i++){
			receiveArray[i]=array[i];
		}
		receiveSize=size;
		
		//受信データの処理
		if(size!=4){
			return;
		}
		
		enum{
			UP_BTN,
			DOWN_BTN,
			RIGHT_BTN,
			LEFT_BTN,
			TRIANGLE_BTN,
			CROSS_BTN,
			CIRCLE_BTN,
			KILL_BTN,
		};
		enum{
			L1_BTN,
		};
		
		if(genBoolFromButtonBit(array[2],KILL_BTN)){
			base::turnEmerg();
			return;
		}
		/*
		const float rotation=byte2floatMotorOutput(array[1])*0.35;
		const float base=0.0*0.3;
		run::setMove(base+rotation,base-rotation);*/
		run::setUserRotateController(byte2floatMotorOutput(array[1]));
		
		run::setUserAdd(byte2floatMotorOutput(array[0]));//!!!!!!!!!!!!!!!
		
		const float upDownMult=genBoolFromButtonBit(array[3],L1_BTN)?3.0:1.0;
		if(genBoolFromButtonBit(array[2],UP_BTN)){
			ajust.up(upDownMult);
		}
		if(genBoolFromButtonBit(array[2],DOWN_BTN)){
			ajust.down(upDownMult);
		}
		if(genBoolFromButtonBit(array[2],TRIANGLE_BTN)){
			ajust.next();
		}
		if(genBoolFromButtonBit(array[2],CROSS_BTN)){
			ajust.back();
		}
		
		
		if(genBoolFromButtonBit(array[2],CIRCLE_BTN)){
			//run::targetDeg=base::TARGET_DEG_INIT;
			run::controlSum=0.0;
			//run::resetGyroAndPid();
		}
		//run::setTargetDeg(targetDeg);
		//printLcd(0,1,rob::flt(targetDeg));
	}
	void ifReceiveFromOtherMachine(uint8_t *array,uint16_t size){
		if(size!=1){
			return;
		}
		kantoStatus=(char)array[0];
	}
	float byte2floatMotorOutput(const uint8_t source){
		using namespace rob::arduino;
		const uint8_t ZERO_VAL=128;
		const uint8_t ZERO_RANGE=80;//+-
		
		if(source<(ZERO_VAL-ZERO_RANGE)){
			//ゼロでないマイナス範囲の値の時！
			return map<float>(source,0,(ZERO_VAL-ZERO_RANGE),-1.0,0);
		}else if((ZERO_VAL+ZERO_RANGE)<source){
			//ゼロでないプラス範囲の値の時！	
			return map<float>(source,(ZERO_VAL+ZERO_RANGE),255,0,1.0);
		}else{
			return 0;
		}
	}
	bool genBoolFromButtonBit(const uint8_t source,const uint8_t shift){
		return (bool)(0x1&(source>>shift));
	}
	
	void setupCom(){
		xbee.attach(callback(ifReceiveFromController));
		otherMachine.attach(callback(ifReceiveFromOtherMachine));
		printLcd(0,0,"o");
	}
	void loopCom(){
		static rob::regularC_ms printLcdTime(400);
		if(printLcdTime){
			printLcd(1,0,rob::flt(run::control));
			ajust.print();
		}
		
		static rob::regularC_ms sendToKantoTime(100);
		if(sendToKantoTime){
			if(kantoStatus!='S'){
				sendToKantoByStatus();
			}else{
				sendToKantoByChar(kantoStatus);//'S'
			}
			//pc.printf("true isKantoOn :%s\n",isKantoOn?"true":"false");
		}
		
	}
	void printReceive(){
		pc.printf("size:%3d",receiveSize);
		for(int i=0;i<receiveSize;i++){
			pc.printf(" %d",receiveArray[i]);
		}
		pc.printf("\n");
	}
	uint8_t calcStrLen(const char str[]){
		uint8_t i=0;
		for(;i<MAX_LCD_STRLEN;i++){
			if(str[i]=='\0'){
				break;
			}
		}
		return i+1;
	}
	void sendToKantoByStatus(){
		bool isGood=run::isGoodDeg();
		/*static bool preIsGood=false;
		static bool isFirst=true;
		if(preIsGood==isGood && !isFirst){
			return;
		}
		isFirst=false;
		preIsGood=isGood;*/
		
		uint8_t c;
		if(isGood){
			c='N';
		}else{
			c='R';
		}
		sendToKantoByChar(c);
		return;
	}
	void sendToKantoByChar(char c){
		static char preSend='\0';
		static rob::timesC forceSendTime(30);
		if(preSend!=c || forceSendTime){
			//pc.printf("kanto send send :%c\n",c);
			kanto.send((uint8_t*)&c,1);
			preSend=c;
		}
		//pc.printf("kanto request send :%c\n",c);
	}
		
	void printLcd(const uint8_t cow,const uint8_t row,const char str[]){
		const uint8_t arrayLen=calcStrLen(str)+2;
		static uint8_t array[MAX_LCD_STRLEN]={};
		array[0]=cow;
		array[1]=row;
		for(uint8_t i=2;i<arrayLen;i++){
			array[i]=str[i-2];
		}
		xbee.send(array,arrayLen);
	}
	
	void sendToOtherMachine(char c ){
		otherMachine.send((uint8_t*)&c,1);
	}
	
	void setKantoStatusAndTellOtherMachine(char c){
		kantoStatus=c;
		sendToOtherMachine(c);
	}
}

namespace checker{
	void printToLcd(){
		static int printRCont=0;
		if(base::isEmerg()){
			com::printLcd(15,0,"E");
		}else{
			com::printLcd(15,0," ");
		}
		if(printRCont!=-1){
			if(printRCont<10){
				com::printLcd(14,0,"R");
				printRCont++;
			}else{
				com::printLcd(14,0," ");
				printRCont=-1;
			}
		}
			
	}
	
	void loopChecker(){
		//static float maxDeg=0.0;
		//static bool noChecked=true;
		static rob::regularC_ms controlTime(500);
		
		if(!controlTime){
			return;
		}
		/*
		if(maxDeg>50.0 && noChecked){
			noChecked=false;
			base::setEmerg(false);
		}
		if(maxDeg<deg::get()){
			maxDeg=deg::get();
		}*/
		printToLcd();
	}
}


namespace vl{	
/*
ajustFloat ajustFloatArray[]={
		ajustFloat("t",&run::targetDeg,0.05),
		ajustFloat("P",&run::degGainP,0.0001),
		ajustFloat("I",&run::degGainI,0.0001),
		ajustFloat("D",&run::degGainD,0.0000001),
		ajustFloat("p",&run::targetDegGainP,0.000001),
		ajustFloat("i",&run::targetDegGainI,0.0000001),
		ajustFloat("d",&run::targetDegGainD,0.00000001),
		ajustFloat("m",&run::targetDegBaseChangeMult,0.001),
	};*/

		//TX RX
	Serial valueLinkRawSerial(PA_0,PA_1,115200);
	valueLinkCore valueLink(valueLinkRawSerial);
	ajustFloatVL array[]={
		ajustFloatVL(valueLink.refManager(),"targetDeg",&run::targetDeg,0.05),
		ajustFloatVL(valueLink.refManager(),"degGainP",&run::degGainP,0.0001),
		ajustFloatVL(valueLink.refManager(),"degGainI",&run::degGainI,0.0001),
		ajustFloatVL(valueLink.refManager(),"degGainD",&run::degGainD,0.0000001),
		ajustFloatVL(valueLink.refManager(),"targetDegGainP",&run::targetDegGainP,0.000001),
		ajustFloatVL(valueLink.refManager(),"targetDegGainI",&run::targetDegGainI,0.0000001),
		ajustFloatVL(valueLink.refManager(),"targetDegGainD",&run::targetDegGainD,0.00000001),
	};
	
	void callbackListener(char c){
		//pc.printf("ALL receive %c\n",c);
		switch (c){
			case 'U':
			run::setUserAdd(-1.0);
			break;
			
			case 'D':
			run::setUserAdd(1.0);
			break;
			
			case 'R':
			run::setUserRotateSumaho(1.0);
			break;
			
			case 'L':
			run::setUserRotateSumaho(-1.0);
			break;
			
			case '0':
			run::setUserRotateSumaho(0.0);
			break;
			
			case 'N':
			case 'S':
			//pc.printf("NS receive %c\n",c);
			com::setKantoStatusAndTellOtherMachine(c);
			break;
			
			default:break;
		}
	}
	
	void setupVL(){
		valueLink.setup();
		valueLink.attachBtn(callback(callbackListener));
	}
	void loopVL(){
		valueLink.loop();
	}
}

int main(){
	rob::regularC_ms printInterval(10);
	com::setupCom();
	run::setupRun();
	deg::setupDeg();
	vl::setupVL();
	
	com::printLcd(0,0,"hello");
	
	//This is a test code
	while(true){
		deg::loopDeg();
		axisX::loopAxisX();
		if(printInterval){
			//com::printReceive();
			//pc.printf("enc2:%6d ",rob::rotaryEncoder2.read());
			run::printDeg();
		}
		run::loopRun();
		com::loopCom();
		checker::loopChecker();
		vl::loopVL();
	}
	
    return 0;
}