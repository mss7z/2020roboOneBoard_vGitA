#include "base.hpp"

namespace base{
	bool isEmergVal=true;
	
}


namespace deg{

extern rob::a_imu03a &imu;
extern rob::aRotaryEncoder &rorycon;

rob::a_imu03a &imu=rob::imu03a;
rob::aRotaryEncoder &rorycon=rob::rotaryEncoder2;

float max=0.0;
//float deg=0.0;
float rawDegRorycon=0.0,degRorycon=0.0,degImu=0.0,accelDeg=0.0,gyroDeg=0.0;

float calcAccelDeg();
void calcDegByImu();
void calcDegByRorycon();


float calcAccelDeg(){
	return (180.0/M_PI)*atanf(imu.accelY.getG()/(-imu.accelX.getG()));
}
void calcDegByImu(){
	//static rob::fromPre_sec realOutputTime;
	
	accelDeg=0.01*(calcAccelDeg()+34.0)+0.99*accelDeg;
	//gyroDeg=imu.gyroZ.getDeg();
	//realOutputTimeVal=realOutputTime.get();
	//定数倍はなぜかずれるから
	gyroDeg+=1.574*imu.gyroZ.getDDeg()*(CALC_DEG_IMU_INTERVAL/1000000.0);
	if(gyroDeg<0){
		gyroDeg=0;
	}
	degImu=0.5*gyroDeg+0.5*accelDeg;
}
/*
void calcDegByRorycon(){
	const int pulsePerRevolution=2048*4;
	const int VALS_LEN=50;
	static int vals[VALS_LEN]={0};
	static int valsIndex=0;
	if(rorycon.read()>0){
		rorycon.set(0);
	}
	vals[valsIndex]=rorycon.read();
	valsIndex++;
	if(valsIndex>=VALS_LEN){
		valsIndex=0;
	}
	int sum=0;
	for(int i=0;i<VALS_LEN;i++){
		sum+=vals[i];
	}
	
	deg=-(360.0*((float)sum/VALS_LEN))/pulsePerRevolution;
}
*/
void calcDegByRorycon(){
	if(rorycon.read()>0){
		rorycon.set(0);
	}
	const double pulsePerRevolution=2048*4;
	float calcK=0.02;
	//calcK*=abs(rorycon.read()-raw)*0.5;
	rawDegRorycon=-1.08523*360.0*(rorycon.read()/pulsePerRevolution);
	//定数倍はなぜかずれるから
	degRorycon=(rawDegRorycon*(1.0-calcK))+rawDegRorycon*calcK;
}


void loopDeg(){
	static rob::regularC_us calcDegTime(CALC_DEG_INTERVAL);
	static rob::delta<float> rpm(CALC_DEG_INTERVAL/1000000.0);
	if(calcDegTime){
		calcDegByRorycon();
		float rpmVal=abs((rpm.f(degRorycon)*60.0)/360.0);
		if(max<rpmVal){
			max=rpmVal;
		}
	}
	static rob::regularC_us calcDegImuTime(CALC_DEG_IMU_INTERVAL);
	if(calcDegImuTime){
		calcDegByImu();
	}
	static rob::regularC_us calcImuOffsetTime(CALC_DEG_IMU_OFFSET_INTERVAL);
	static rob::delta<float> ddegCalcer(CALC_DEG_IMU_OFFSET_INTERVAL/1000000.0);
	if(calcImuOffsetTime){
		//imu.gyroZ.calcOffsetByTrueDdeg(ddegCalcer.f(degRorycon),0.0001);
	}
}
}

namespace axisX{
rob::aRotaryEncoder &displacementEnc=rob::rotaryEncoder1;

float valX=0.0;

void calcValx(){
	const int pulsePerRevolution=125*4;
	static float raw=0.0;
	float calcK=0.002;
	//calcK*=abs(rorycon.read()-raw)*0.5;
	raw=(raw*(1.0-calcK))+displacementEnc.read()*calcK;
	valX=(80.0*3.1415926535*raw)/pulsePerRevolution;
}

void loopAxisX(){
	static rob::regularC_us calcValxTime(CALC_VALX_INTERVAL);
	if(calcValxTime){
		calcValx();
	}
}

}
