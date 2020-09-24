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

rob::aXbeeCom adb(rob::xbeeCore,rob::xbee64bitAddress(0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36));

Serial pc(USBTX,USBRX);
DigitalOut led(LED1);

rob::aTB6643KQ &motor1=rob::tb6643kq_md3;
rob::aTB6643KQ &motor2=rob::tb6643kq_md4;

int main(){
	
	//This is a test code
	while(true){
		pc.printf("Hello, Mbed! led is %d\n",(int)led);
		led=!led;
		wait(0.5);
		
		motor1=0.5;
		motor2=-0.3;
	}
	
    return 0;
}