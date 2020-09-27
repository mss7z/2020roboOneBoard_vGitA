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

namespace a_imu03a_internal{
class imu03aSPI{
	private:
	SPI sp;
	DigitalOut cs;
	
	public:
	imu03aSPI(
		PinName mosi,
		PinName miso,
		PinName sclk,
		PinName csPin,
	);
	void start(){sp=0;}
	int write(const int val){return sp.write(val);}
	void end(){cs=1;}
};
class imu03aSetting{
	private:
	imu03aSPI &com;
	
	public:
	imu03aSetting(imu03aSPI&);
	void resetModule();
	bool isNormal();
};
class imu03aGyroAccelBase{
	private:
	const uint8_t LbitReg;
	const uint8_t HbitReg;
	imu03aSPI &com;
	protected:
	imu03aGyroAccelBase(imu03aSPI&,const uint8_t l,const uint8_t h);
	uint16_t getRawVal();
};
	
class imu03aGyro:
	public imu03aGyroAccelBase
{
	private:
	uint16_t offsetRawVal;
	
	int16_t getOffset();
	int16_t getRawVal();
	float rawVal2DDeg(const int16_t);
	
	public:
	imu03aGyro(imu03aSPI &c,const uint8_t l,const uint8_t h):
		imu03aGyroAccelBase(c,l,h){}
	float getDDeg();
	void resetOffset();
};
class imu03aAccel:
	public imu03aGyroAccelBase
{
	private:
	
	float rawVal2G(const uint16_t);
	
	public:
	imu03aGyro(imu03aSPI &c,const uint8_t l,const uint8_t h):
		imu03aGyroAccelBase(c,l,h){}
	float getG();
	float getMperS2();
};
	
class a_imu03a{
	private:
	imu03aSPI com;
	imu03aSetting setting;
	
	public:
	imu03aGyro gyroX;
	imu03aGyro gyroY;
	imu03aGyro gyroZ;
	imu03aAccel accelX;
	imu03aAccel accelY;
	imu03aAccel accelZ;
	
	a_imu03aimu03aSPI(
		PinName mosi,
		PinName miso,
		PinName sclk,
		PinName csPin,
	);
	void resetModule();
	void isNormal(){return setting.isNormal();}
};

namespace old{
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
}//namespace old


}//namespace a_imu03a_internal
using a_imu03a_internal::a_imu03a;
}//namespace rob


#endif