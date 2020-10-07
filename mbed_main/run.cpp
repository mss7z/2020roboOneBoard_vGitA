#include "run.hpp"

namespace run{
	
motor motorL(rob::tb6643kq_md3,-1.0);
motor motorR(rob::tb6643kq_md4,1.0);

rob::a_imu03a &imu=rob::imu03a;
rob::aRotaryEncoder &rorycon=rob::rotaryEncoder2;

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


void pidAndOutput(){
	static rob::regularC_us outputTime(CONTROL_CYCLE_TIME);
	
	if(!outputTime){
		return;
	}
	if(base::isEmerg()){
		return;
	}
	
	/*現在角度算出*/
	//calcDegByImu();
	calcDegByRorycon();
	
	/*const float */control=degPid.calc(deg);//+ddegPid.calc(imu.gyroZ.getDDeg());
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
		}*/
		
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
	}*/
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
	gyroDeg=deg=total/20;*/
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
	/*35.7,28.3,34.9,35.7,34.5,30.7,31.2,29.5,33.2,33.6*/
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

}//namespace run
