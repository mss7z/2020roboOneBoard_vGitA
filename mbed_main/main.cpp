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

rob::aXbeeCom xbee(rob::xbeeCore,rob::xbee64bitAddress(0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36));

Serial pc(USBTX,USBRX,115200);
DigitalOut led(LED1);

rob::aTB6643KQ &motor1=rob::tb6643kq_md3;
rob::aTB6643KQ &motor2=rob::tb6643kq_md4;

namespace com{
	uint8_t receiveArray[255]={0};
	uint16_t receiveSize=0;
	
	void setupCom();
	void ifReceiveFromController(uint8_t*,uint16_t);
	
	
	void setupCom(){
		xbee.attach(callback(ifReceiveFromController));
	}
	void ifReceiveFromController(uint8_t *array,uint16_t size){
		for(int i=0;i<size;i++){
			receiveArray[i]=array[i];
		}
		receiveSize=size;
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
	com::setupCom();
	
	
	//This is a test code
	while(true){
		pc.printf("Hello, Mbed! led is %d\n",(int)led);
		led=!led;
		wait(0.1);
		
		com::printReceive();
		
	}
	
    return 0;
}