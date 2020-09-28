#include "mbed.h"

#define ENABLE_tb6643kq_md1
#define ENABLE_tb6643kq_md2
#define ENABLE_tb6643kq_md3
#define ENABLE_tb6643kq_md4
#define ENABLE_rotatyEncoder1
#define ENABLE_rotatyEncoder2
#define ENABLE_imu03a
#define ENABLE_xbeeCore
#define VAL_xbeeCore_serialSpeed 38400

#include "2020roboOneBoardLib/lib.hpp"

namespace base{
	bool isEmergVal=false;
	bool setEmerg(const bool val){
		return isEmergVal=val;
	}
	bool isEmerg(){
		return isEmergVal;
	}
	bool turnEmerg(){
		return isEmergVal=!isEmergVal;
	}
}

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
	
	//const rob::pidGain gain={0.000102,0.000,0.00001};
	//const rob::pidGain degGain={0.001912,0.000,0.00003};
	const rob::pidGain degGain={0.004812,0.00001,0.0000};
	rob::aPid<float> degPid(degGain,CONTROL_CYCLE_TIME_SEC);
	
	//const rob::pidGain ddegGain={0.00022,0.00001,0.0000};
	const rob::pidGain ddegGain={0.000,0000,0.0000};
	rob::aPid<float> ddegPid(ddegGain,CONTROL_CYCLE_TIME_SEC);
	
	//calc(control)の時
	//const rob::pidGain targetDegGain={0.00,0.001,0.00000};
	//calc(control)+1.5
	const rob::pidGain targetDegGain={0.0000,0.000001,0.00000};
	rob::aPid<float> targetDegPid(targetDegGain,CONTROL_CYCLE_TIME_SEC);
	
	
	
	
	rob::a_imu03a &imu=rob::imu03a;
	float accelDeg=0.0,gyroDeg=0.0,deg=0.0;
	float realOutputTimeVal=0.0;
	
	
	//内部
	void pidAndOutput();
	float calcAccelDeg();
	
	//外部
	void setMove(const float valL,const float valR);
	void setTargetDeg(const float deg);
	void resetGyroAndPid();
	void printDeg();
	void isEmergency(bool);
	
	void pidAndOutput(){
		static rob::regularC_us outputTime(CONTROL_CYCLE_TIME);
		
		static rob::fromPre_sec realOutputTime;
		
		if(!outputTime){
			return;
		}
		if(base::isEmerg()){
			return;
		}
		
		/*現在角度算出*/
		accelDeg=0.01*calcAccelDeg()+0.99*accelDeg;
		//gyroDeg=imu.gyroZ.getDeg();
		realOutputTimeVal=realOutputTime.get();
		//定数倍はなぜかずれるから
		gyroDeg+=1.1365*imu.gyroZ.getDDeg()*(realOutputTimeVal);
		deg=0.5*gyroDeg+0.5*accelDeg;
		
		
		const float controll=degPid.calc(deg)+ddegPid.calc(imu.gyroZ.getDDeg());
		motorL.output(controll);
		motorR.output(controll);
		
		/*if(imu.gyroZ.getDDeg()<0.00){
			degPid.set(degPid.read()+0.002);
		}else{
			degPid.set(degPid.read()-0.002);
		}*/
		degPid.set(targetDegPid.calc(controll));
		//degPid.set(targetDegPid.calc(imu.gyroZ.getDDeg()));
	}
	float calcAccelDeg(){
		return (180.0/M_PI)*atanf(imu.accelY.getG()/(-imu.accelX.getG()));
	}
	
	void setMove(const float valL,const float valR){
		motorL.setBase(valL);
		motorR.setBase(valR);
	}
	void setTargetDeg(const float deg){
		//degPid.set(deg);
	}
	void resetGyroAndPid(){
		degPid.reset();
		ddegPid.reset();
		imu.resetModule();
		float total=0.0;
		for(int i=0;i<20;i++){
			total+=calcAccelDeg();
			wait_us(625);
		}
		gyroDeg=deg=total/20;
		//imu.gyroZ.startDeg();
	}
	void printDeg(){
		pc.printf("realT: %sus ax:%s ay:%s dz:%6s ",rob::flt(realOutputTimeVal*1000000.0),rob::flt(imu.accelX.getG()),rob::flt(imu.accelY.getG()),rob::flt(imu.gyroZ.getDDeg()));
		pc.printf("deg:%s gyroDeg:%s accelDeg:%s  tagDeg:%s\n",rob::flt(deg),rob::flt(gyroDeg),rob::flt(accelDeg),rob::flt(degPid.read()));
	}
	
	void setupRun(){
		resetGyroAndPid();
		//setTargetDeg(1.3);
		ddegPid.set(0.0);
		/*35.7,28.3,34.9,35.7,34.5,30.7,31.2,29.5,33.2,33.6*/
		//gyro.setDeg(-32.73);
	}
	void loopRun(){
		pidAndOutput();
		if(base::isEmerg()){
			motorL.stop();
			motorR.stop();
		}
	}
}

namespace com{
	rob::aXbeeCom xbee(rob::xbeeCore,rob::xbee64bitAddress(0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0xF1));
	
	uint8_t receiveArray[255]={0};
	uint16_t receiveSize=0;
	
	float targetDeg=2.0;
	
	//内部
	void ifReceiveFromController(uint8_t*,uint16_t);
	float byte2floatMotorOutput(uint8_t);
	bool genBoolFromButtonBit(uint8_t,uint8_t);
	
	//外部
	void setupCom();
	
	void ifReceiveFromController(uint8_t *array,uint16_t size){
		for(int i=0;i<size;i++){
			receiveArray[i]=array[i];
		}
		receiveSize=size;
		
		//受信データの処理
		if(size!=3){
			return;
		}
		
		enum{
			FORWARD_BTN,
			REVERSE_BTN,
			DEG_UP_BTN,
			DEG_DOWN_BTN,
			DEG_ZERO_BTN,
			KILL_BTN,
		};
		
		if(genBoolFromButtonBit(array[2],KILL_BTN)){
			base::turnEmerg();
			return;
		}
		
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
		
		
		if(genBoolFromButtonBit(array[2],DEG_UP_BTN)){
			targetDeg+=0.020;
		}
		if(genBoolFromButtonBit(array[2],DEG_DOWN_BTN)){
			targetDeg-=0.020;
		}
		if(genBoolFromButtonBit(array[2],DEG_ZERO_BTN)){
			targetDeg=2.0;
			run::resetGyroAndPid();
		}
		run::setTargetDeg(targetDeg);
		
	}
	float byte2floatMotorOutput(const uint8_t source){
		using namespace rob::arduino;
		const uint8_t ZERO_VAL=128;
		const uint8_t ZERO_RANGE=50;//+-
		
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
	void printReceive(){
		pc.printf("size:%3d",receiveSize);
		for(int i=0;i<receiveSize;i++){
			pc.printf(" %d",receiveArray[i]);
		}
		pc.printf("\n");
	}
}

int main(){
	rob::regularC_ms printInterval(100);
	com::setupCom();
	run::setupRun();
	
	//This is a test code
	while(true){
		if(printInterval){
			//com::printReceive();
			run::printDeg();
		}
		run::loopRun();
	}
	
    return 0;
}