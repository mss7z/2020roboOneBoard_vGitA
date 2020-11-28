#include "base.hpp"

namespace base{
	bool isEmergVal=false;
	
}


namespace deg{

extern rob::a_imu03a &imu;
extern rob::aRotaryEncoder &rorycon;

rob::a_imu03a &imu=rob::imu03a;
rob::aRotaryEncoder &rorycon=rob::rotaryEncoder2;

float max=0.0;
//float deg=0.0;
float rawDegRorycon=0.0,degRorycon=0.0,degAccel=0.0,degGyro=0.0;

const float pulsePerRevolution=2048*4;
const float roryconReadMult=1.;

float calcRoryconToDeg(int);
int calcDegToRorycon(float);
	

float calcAccelDeg();
void calcDegByImu();
void calcDegByRorycon();


float calcRoryconToDeg(int v){
	return -roryconReadMult*360.0*(v/pulsePerRevolution);
}
int calcDegToRorycon(float v){
	return (int)(-pulsePerRevolution*(v/(360.0*roryconReadMult)));
}
float calcAccelDeg(){
	return (180.0/M_PI)*atanf(imu.accelY.getG()/(-imu.accelX.getG()));
}
void calcDegByImu(){
	//static rob::fromPre_sec realOutputTime;
	
	degAccel=0.01*(calcAccelDeg()+34.0)+0.99*degAccel;
	//gyroDeg=imu.gyroZ.getDeg();
	//realOutputTimeVal=realOutputTime.get();
	//定数倍はなぜかずれるから
	degGyro+=1.574*imu.gyroZ.getDDeg()*(CALC_DEG_IMU_INTERVAL/1000000.0);
	/*if(gyroDeg<0){
		gyroDeg=0;
	}*/
	const float calcK=0.001;
	degGyro=calcK*degAccel+degGyro*(1.0-calcK);
	//degImu=0.5*gyroDeg+0.5*accelDeg;
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
	/*if(rorycon.read()>0){
		rorycon.set(0);
	}*/
	float calcK=0.02;
	//calcK*=abs(rorycon.read()-raw)*0.5;
	//rawDegRorycon=-1.08523*360.0*(rorycon.read()/pulsePerRevolution);
	rawDegRorycon=calcRoryconToDeg(rorycon.read());
	//定数倍はなぜかずれるから
	degRorycon=(degRorycon*(1.0-calcK))+rawDegRorycon*calcK;
	
	const float calcKAccel=0.001;
	degRorycon=calcKAccel*degAccel+(1.0-calcKAccel)*degRorycon;
}

void setupDeg(){
	for(int i=0;i<5000;i++){
		calcDegByImu();
	}
	degGyro=degAccel;
	rorycon.set(calcDegToRorycon(degAccel));
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
		imu.gyroZ.calcOffsetByTrueDdeg(ddegCalcer.f(rawDegRorycon)/1.574,0.0001);
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
