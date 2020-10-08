#include "mbed.h"


#include "base.hpp"
#include "run.hpp"
/*
namespace run{
	class motor{
		private:
		rob::aTB6643KQ &mtInst;
		float baseOutput;
		const float constMult;
		public:
		motor(rob::aTB6643KQ &mt,const float cm):mtInst(mt),baseOutput(0.0),constMult(cm){}
		void setBase(const float val){baseOutput=val;}
		void output(const float val){mtInst=constMult*(val+baseOutput);}
		void stop(){mtInst=0.0;}
	};
	motor motorL(rob::tb6643kq_md3,-1.0);
	motor motorR(rob::tb6643kq_md4,1.0);
	
	const int CONTROL_CYCLE_TIME=5000;//us
	const float CONTROL_CYCLE_TIME_SEC=CONTROL_CYCLE_TIME/1000000.0;//sec
	const float PID_OPERATION_MAX=1.0,PID_OPERATION_MIN=-1.0;
	//const rob::pidGain gain={0.000102,0.000,0.00001};
	//const rob::pidGain degGain={0.001912,0.000,0.00003};
	//const rob::pidGain degGain={0.001812,0.00001,0.0000};
	//pid P:0.00181200 I:0.01714998 D:0.00010000
	//そこそこ？float degGainP=0.001812;float degGainI=0.01479;float degGainD=0.000088;
	//float degGainP=0.003912;float degGainI=0.03189;float degGainD=0.0000929;//体育間1
	//float degGainP=0.006811;float degGainI=0.00949;float degGainD=0.0000617;//体育館2
	//float degGainP=0.003211;float degGainI=0.01038;float degGainD=0.0000721;//体育館2
	float degGainP=0.00821;float degGainI=0.01748;float degGainD=0.0000721;//0A07
	
	
	//float degGainP=0.001812;float degGainI=0.000013;float degGainD=0.00;
	rob::aPid<float> degPid(degGainP,degGainI,degGainD,CONTROL_CYCLE_TIME_SEC,PID_OPERATION_MAX,PID_OPERATION_MIN);
	
	//const rob::pidGain ddegGain={0.00022,0.00001,0.0000};
	//const rob::pidGain ddegGain={0.000,0000,0.0000};
	//rob::aPid<float> ddegPid(ddegGain,CONTROL_CYCLE_TIME_SEC);
	
	//calc(control)の時
	//const rob::pidGain targetDegGain={0.00,0.001,0.00000};
	//calc(controlSum)の時
	//const rob::pidGain targetDegGain={0.00000005,0.00,0.0000003};
	//calc(control)+1.5
	//const rob::pidGain targetDegGain={0.0000,0.000001,0.00000};
	//float targetDegGainP=0.00000005;float targetDegGainI=0.0;float targetDegGainD=0.0000003;//0A07最初
	float targetDegGainP=0.0000000;float targetDegGainI=0.0;float targetDegGainD=0.000000;//ZERO
	//float targetDegGainP=0.00000575;float targetDegGainI=0.00000072;float targetDegGainD=0.00000264;//0A07最初
	rob::aPid<float> targetDegPid(targetDegGainP,targetDegGainI,targetDegGainD,CONTROL_CYCLE_TIME_SEC,PID_OPERATION_MAX,PID_OPERATION_MIN);
	
	rob::a_imu03a &imu=rob::imu03a;
	rob::aRotaryEncoder &rorycon=rob::rotaryEncoder2;
	float accelDeg=0.0,gyroDeg=0.0,deg=0.0;
	float realOutputTimeVal=0.0;
	
	float userControll=0.0;
	float control=0.0;
	float controlSum=0.0;
	
	float targetDeg=base::TARGET_DEG_INIT;
	
	
	//内部
	void pidAndOutput();
	float calcAccelDeg();
	void calcDegByImu();
	void calcDegByRorycon();
	
	//外部
	void setMove(const float valL,const float valR);
	void setTargetDeg(const float deg);
	void resetGyroAndPid();
	void printDeg();
	void isEmergency(bool);
	
	void pidAndOutput(){
		static rob::regularC_us outputTime(CONTROL_CYCLE_TIME);
		
		if(!outputTime){
			return;
		}
		if(base::isEmerg()){
			return;
		}
		
		//現在角度算出
		//calcDegByImu();
		calcDegByRorycon();
		
		control=degPid.calc(deg);//+ddegPid.calc(imu.gyroZ.getDDeg());
		motorL.output(control);
		motorR.output(control);
		
		if(-0.9<control && control<0.9){
			controlSum+=control;
			/*const float controllSumLimit=15.0;
			if(controllSum>controllSumLimit){
				controllSum=0.0;
				targetDeg+=0.05;
			}else if(controllSum<-controllSumLimit){
				controllSum=0.0;
				targetDeg-=0.05;
			}*
			
			const float controlSumLimit=35.0;
			if(controlSum>controlSumLimit){
				controlSum=controlSumLimit;
			}else if(controlSum<-controlSumLimit){
				controlSum=-controlSumLimit;
			}
			targetDeg-=targetDegPid.calc(controlSum);
		}
		
		/*if(imu.gyroZ.getDDeg()<0.00){
			degPid.set(degPid.read()+0.002);
		}else{
			degPid.set(degPid.read()-0.002);
		}*
		//degPid.set(targetDegPid.calc(controll)+userControll);
		//degPid.set(targetDegPid.calc(controllSum)+userControll);
		//degPid.set(targetDegPid.calc(imu.gyroZ.getDDeg()));
	}
	float calcAccelDeg(){
		return (180.0/M_PI)*atanf(imu.accelY.getG()/(-imu.accelX.getG()));
	}
	void calcDegByImu(){
		static rob::fromPre_sec realOutputTime;
		
		accelDeg=0.01*calcAccelDeg()+0.99*accelDeg;
		//gyroDeg=imu.gyroZ.getDeg();
		realOutputTimeVal=realOutputTime.get();
		//定数倍はなぜかずれるから
		gyroDeg+=1.1365*imu.gyroZ.getDDeg()*(realOutputTimeVal);
		deg=0.5*gyroDeg+0.5*accelDeg;
	}
	void calcDegByRorycon(){
		const int pulsePerRevolution=2048*4;
		if(rorycon.read()>0){
			rorycon.set(0);
		}
		deg=-(360.0*rorycon.read())/pulsePerRevolution;
	}
	
	void setMove(const float valL,const float valR){
		motorL.setBase(valL);
		motorR.setBase(valR);
	}
	float degAdd=0.0;
	void setTargetDeg(const float deg){
		degPid.set(degAdd+deg);
		//userControll=deg;
	}
	void setTargetDegAdd(const float deg){
		degAdd=deg;
	}
	void resetGyroAndPid(){
		degPid.reset();
		//ddegPid.reset();
		/*imu.resetModule();
		float total=0.0;
		for(int i=0;i<20;i++){
			total+=calcAccelDeg();
			wait_us(625);
		}
		gyroDeg=deg=total/20;*
		//imu.gyroZ.startDeg();
		
	}
	void printDeg(){
		using namespace rob;
		//pc.printf("realT: %sus ax:%s ay:%s dz:%6s ",rob::flt(realOutputTimeVal*1000000.0),rob::flt(imu.accelX.getG()),rob::flt(imu.accelY.getG()),rob::flt(imu.gyroZ.getDDeg()));
		//pc.printf("deg:%s gyroDeg:%s accelDeg:%s  tagDeg:%s\n",rob::flt(deg),rob::flt(gyroDeg),rob::flt(accelDeg),rob::flt(degPid.read()));
		pc.printf("pid P:%s I:%s D:%s  deg:%s",flt(degGainP,8),flt(degGainI,8),flt(degGainD,8),flt(deg));
		pc.printf("  controlSum:%s",flt(controlSum));
		
		pc.printf("\n");
	}
	
	void setupRun(){
		resetGyroAndPid();
		//setTargetDeg(1.3);
		//ddegPid.set(0.0);
		targetDegPid.set(0.0);
		//degPid.set(base::TARGET_DEG_INIT);
		//35.7,28.3,34.9,35.7,34.5,30.7,31.2,29.5,33.2,33.6
		//gyro.setDeg(-32.73);
	}
	void loopRun(){
		pidAndOutput();
		if(base::isEmerg()){
			motorL.stop();
			motorR.stop();
		}
		degPid.setGain(degGainP,degGainI,degGainD);
		degPid.set(targetDeg);
	}
}*/

namespace com{
	rob::aXbeeCom xbee(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0xF1));
	
	uint8_t receiveArray[255]={0};
	uint16_t receiveSize=0;
	
	const uint8_t MAX_LCD_STRLEN=100;
	
	//内部
	void ifReceiveFromController(uint8_t*,uint16_t);
	float byte2floatMotorOutput(uint8_t);
	bool genBoolFromButtonBit(uint8_t,uint8_t);
	uint8_t calcStrLen(const char []);
	
	//外部
	void setupCom();
	void printLcd(const uint8_t col,const uint8_t row, const char str[]);
	
	
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
		void printAll(){printLcd(0,1,name);printLcd(1,1,"           ");print();}
	};
	ajustFloat ajustFloatArray[]={
		ajustFloat("t",&run::targetDeg,0.05),
		ajustFloat("P",&run::degGainP,0.0001),
		ajustFloat("I",&run::degGainI,0.0001),
		ajustFloat("D",&run::degGainD,0.0000001),
		ajustFloat("p",&run::targetDegGainP,0.00000001),
		ajustFloat("i",&run::targetDegGainI,0.00000001),
		ajustFloat("d",&run::targetDegGainD,0.00000001),
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
		float forwardVal=0;
		//0L 1R
		if(genBoolFromButtonBit(array[2],FORWARD_BTN)){
			forwardVal+=0.3;
		}
		if(genBoolFromButtonBit(array[2],REVERSE_BTN)){
			forwardVal-=0.3;
		}
		
		const float valL=byte2floatMotorOutput(array[0])+forwardVal;
		const float valR=byte2floatMotorOutput(array[1])+forwardVal;
		
		run::setMove(valL,valR);
		*/
		
		const float rotation=byte2floatMotorOutput(array[1])*0.2;
		run::setMove(rotation,-rotation);
		
		run::setTargetDisplacementAdd(byte2floatMotorOutput(array[0])*2.5);
		
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
		ajust.print();
		
		/*if(genBoolFromButtonBit(array[2],DEG_UP_BTN)){
			targetDeg+=0.040;
		}
		if(genBoolFromButtonBit(array[2],DEG_DOWN_BTN)){
			targetDeg-=0.040;
		}*/
		if(genBoolFromButtonBit(array[2],CIRCLE_BTN)){
			//run::targetDeg=base::TARGET_DEG_INIT;
			run::controlSum=0.0;
			//run::resetGyroAndPid();
		}
		//run::setTargetDeg(targetDeg);
		//printLcd(0,1,rob::flt(targetDeg));
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
	}
	void loopCom(){
		static rob::regularC_ms printLcdTime(100);
		if(printLcdTime){
			//printLcd(0,0,rob::flt(run::deg-run::degPid.read()));
			printLcd(0,0,"o");
			printLcd(1,0,rob::flt(run::control));
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
	
}

int main(){
	rob::regularC_ms printInterval(100);
	com::setupCom();
	run::setupRun();
	
	com::printLcd(0,0,"hello");
	
	//This is a test code
	while(true){
		if(printInterval){
			//com::printReceive();
			pc.printf("enc1:%6d ",rob::rotaryEncoder1.read());
			run::printDeg();
		}
		run::loopRun();
		com::loopCom();
	}
	
    return 0;
}