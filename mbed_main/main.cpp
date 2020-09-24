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



Serial pc(USBTX,USBRX,115200);
DigitalOut led(LED1);

namespace run{
	class motor{
		private:
		rob::aTB6643KQ &mtInst;
		float baseOutput;
		public:
		motor(rob::aTB6643KQ &mt):mtInst(mt),baseOutput(0.0){}
		void setBase(const float val){baseOutput=val;}
		void output(const float val){mtInst=val+baseOutput;}
	};
	motor motorL(rob::tb6643kq_md3);
	motor motorR(rob::tb6643kq_md4);
	
	//内部
	void pidAndOutput();
	
	//外部
	void setBase(const float valL,const float valR);
	
	void pidAndOutput(){
		motorL.output(0.0);
		motorR.output(0.0);
	}
	
	void setBase(const float valL,const float valR){
		motorL.setBase(valL);
		motorR.setBase(valR);
	}
	void loopRun(){
		rob::regularC outputTime(50);
		if(outputTime){
			pidAndOutput();
		}
	}
}

namespace com{
	rob::aXbeeCom xbee(rob::xbeeCore,rob::xbee64bitAddress(0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36));
	
	uint8_t receiveArray[255]={0};
	uint16_t receiveSize=0;
	
	//内部
	void ifReceiveFromController(uint8_t*,uint16_t);
	float byte2floatMotorOutput(uint8_t);
	
	//外部
	void setupCom();
	
	void ifReceiveFromController(uint8_t *array,uint16_t size){
		for(int i=0;i<size;i++){
			receiveArray[i]=array[i];
		}
		receiveSize=size;
		
		//受信データの処理
		if(size!=2){
			return;
		}
		//0L 1R
		run::setBase(byte2floatMotorOutput(array[0]),byte2floatMotorOutput(array[1]));
		
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
	
	
	//This is a test code
	while(true){
		if(printInterval){
			com::printReceive();
		}
		run::loopRun();
	}
	
    return 0;
}