#ifndef GYRO_IMU03A_INCLUDE_GUARD
#define GYRO_IMU03A_INCLUDE_GUARD

#include "mbed.h"
#include "useful.hpp"

namespace rob{
/*
	注意
	imu03aは角速度センサーと加速度センサーを搭載していますが
	このclassで読んでいるのはz軸の角速度センサーだけです
	
	つなぎ方
	imu03a 	NUCLEO
	
	Vin -	3V3
	GND - 	GND
	
	SDA -	MOSI
	SCL -	SLCK
	SDO -	MISO
	
	CS -	CS
*/

//extern int moniva;

class a_imu03a{
	private:
	//public:
		SPI sp;
		DigitalOut cs;
		Ticker tc;
		
		static const int deltaT=700;//us
		
		void comS(){
			cs=0;
		}
		void comE(){
			cs=1;
			//wait_us(1);
		}
		double getOffset();
		
		double offsetVal;
		//double getRawDdeg();
		
		double deg;
		void sumDdegP();
		void sumDdegM();
		
		int16_t getRawVal();
		double rawValToDdeg(int16_t);
		
		int whoamiCont;
		bool isNormalVal;
		bool isNormalChecker();
		
		bool isForwardVal;
	public:
		a_imu03a(
			PinName mosi,
			PinName miso,
			PinName sclk,
			PinName csPin,
			bool isForward=true
		);
		void resetModule();
		
		void resetDeg();
		void startDeg();
		void stopDeg();
		
		double getDdeg();
		double getDeg(){return deg;}
		
		void setDeg(double val){deg=val;}
		
		bool isNormal(){return isNormalVal;}
};
}


#endif