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
	motor motorL(rob::tb6643kq_md3,1.0);
	motor motorR(rob::tb6643kq_md4,-1.0);
	
	const int CONTROL_CYCLE_TIME=1;//ms
	
	const rob::pidGain gain={0.000102,0.000,0.000015};
	rob::aPid<float> pid(gain,CONTROL_CYCLE_TIME/1000.0);
	
	rob::a_imu03a &imu=rob::imu03a;
	float deg=0.0;
	
	
	//内部
	void pidAndOutput();
	
	//外部
	void setMove(const float valL,const float valR);
	void setTargetDeg(const float deg);
	void resetGyroAndPid();
	void printDeg();
	void isEmergency(bool);
	
	void pidAndOutput(){
		static rob::regularC outputTime(CONTROL_CYCLE_TIME);
		
		if(!outputTime){
			return;
		}
		if(base::isEmerg()){
			return;
		}
		
		deg+=imu.gyroZ.getDDeg()*(CONTROL_CYCLE_TIME/1000.0);
		const float controll=pid.calc(deg);
		motorL.output(controll);
		motorR.output(controll);
	}
	
	
	void setMove(const float valL,const float valR){
		motorL.setBase(valL);
		motorR.setBase(valR);
	}
	void setTargetDeg(const float deg){
		pid.set(deg);
	}
	void resetGyroAndPid(){
		pid.reset();
		imu.resetModule();
		deg=0.0;
		//gyro.startDeg();
	}
	void printDeg(){
		pc.printf("deg:%s tagDeg:%s\n",rob::flt(deg),rob::flt(pid.read()));
	}
	
	void setupRun(){
		resetGyroAndPid();
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
	
	float targetDeg=0;
	
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
			targetDeg=0.0;
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
	rob::regularC printInterval(100);
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