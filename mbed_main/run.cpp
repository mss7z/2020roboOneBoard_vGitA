#include "run.hpp"


namespace run{



motor motorL(rob::tb6643kq_md3,-1.0);
motor motorR(rob::tb6643kq_md4,1.0);

rob::a_imu03a &imu=rob::imu03a;
rob::aRotaryEncoder &rorycon=rob::rotaryEncoder2;
rob::aRotaryEncoder &displacementEnc=rob::rotaryEncoder1;

//const rob::pidGain gain={0.000102,0.000,0.00001};
//const rob::pidGain degGain={0.001912,0.000,0.00003};
//const rob::pidGain degGain={0.001812,0.00001,0.0000};
//pid P:0.00181200 I:0.01714998 D:0.00010000
//そこそこ？float degGainP=0.001812;float degGainI=0.01479;float degGainD=0.000088;
//float degGainP=0.003912;float degGainI=0.03189;float degGainD=0.0000929;//体育間1
//float degGainP=0.006811;float degGainI=0.00949;float degGainD=0.0000617;//体育館2
//float degGainP=0.003211;float degGainI=0.01038;float degGainD=0.0000721;//体育館2
//float degGainP=0.00821;float degGainI=0.01748;float degGainD=0.0000721;//0A07
//float degGainP=0.00781;float degGainI=0.01538;float degGainD=0.00006769;//0A08
//float degGainP=0.00661;float degGainI=0.03087;float degGainD=0.00016879;//0A08
//float degGainP=0.00621;float degGainI=0.03327;float degGainD=0.00008058;//0A08 good
//float degGainP=0.00571;float degGainI=0.03327;float degGainD=0.00007857;//0A10
//float degGainP=0.00711;float degGainI=0.01707;float degGainD=0.00007857;//0A10
//float degGainP=0.00711;float degGainI=0.01707;float degGainD=0.00009437;//0A13
//float degGainP=0.00561;float degGainI=0.01257;float degGainD=0.00006416;//0A13
//float degGainP=0.01121;float degGainI=0.02057;float degGainD=0.00005516;//0A13
//float degGainP=0.00971;float degGainI=0.01727;float degGainD=0.00005345;//0A16 100us 50times
//float degGainP=0.00471;float degGainI=0.01077;float degGainD=0.00005345;//0A16 300us 50times
//float degGainP=0.00391;float degGainI=0.01077;float degGainD=0.00004225;//0A17 testrun 1719 300us 50times
float degGainP=0.00400;float degGainI=0.00847;float degGainD=0.00005345;//0A17 testrun 1719 300us 50times



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
//float targetDegGainP=0.00000741;float targetDegGainI=0.0;float targetDegGainD=0.000000;//ZERO
//float targetDegGainP=0.00000934;float targetDegGainI=0.00000016;float targetDegGainD=0.00000071;//good
//float targetDegGainP=0.00000934;float targetDegGainI=0.00000000;float targetDegGainD=0.00000071;//good
//float targetDegGainP=0.00000744;float targetDegGainI=0.00000045;float targetDegGainD=0.00000245;//good
//float targetDegGainP=0.00001020;float targetDegGainI=0.00000409;float targetDegGainD=0.00000061;//good
//float targetDegGainP=0.00001005;float targetDegGainI=0.00000421;float targetDegGainD=0.00000041;//good
//float targetDegGainP=0.00003648;float targetDegGainI=0.00000877;float targetDegGainD=0.00000041;//good
//float targetDegGainP=0.00000575;float targetDegGainI=0.00000160;float targetDegGainD=0.00000041;//0a18 good 0919
float targetDegGainP=0.00000575;float targetDegGainI=0.00000129;float targetDegGainD=0.00000041;//0a18 good 0919

//float targetDegGainP=0.00000575;float targetDegGainI=0.00000072;float targetDegGainD=0.00000264;//0A07最初
rob::aPid<float> targetDegPid(targetDegGainP,targetDegGainI,targetDegGainD,CONTROL_CYCLE_TIME_SEC,10,-10);

float accelDeg=0.0,gyroDeg=0.0,deg=0.0;
float realOutputTimeVal=0.0;

float userControll=0.0;
float control=0.0;
float controlSum=0.0;

float targetDeg=base::TARGET_DEG_INIT;
float targetDegAdd=0.0;
float targetDegUser=0.0;

float targetDegBaseChangeMult=0.999;

float displacement=0.0;
float displacementAdd=0.0;


float displacementLast=0.0;

//内部
float getDegDiff();
void pidAndOutput();
float calcAccelDeg();
void calcDegByImu();
void calcDegByRorycon();
void calcDisplacement();


float getDegDiff(){
	return degPid.read()-deg;
}
void pidAndOutput(){
	static rob::regularC_us outputTime(CONTROL_CYCLE_TIME);
	static rob::regularC_us calcDegTime(300);
	static rob::regularC_ms calcTargetDegTime(100);
	
	
	
	if(calcDegTime){
		calcDegByRorycon();
	}
	if(base::isEmerg()){
		return;
	}
	if(calcTargetDegTime){
		targetDegAdd=-targetDegPid.calc(displacement);
		targetDeg=targetDeg*targetDegBaseChangeMult+(targetDeg+targetDegAdd)*(1.0-targetDegBaseChangeMult);
	}
	
	if(!outputTime){
		return;
	}
	/*現在角度算出*/
	//calcDegByImu();
	calcDegByRorycon();
	calcDisplacement();
	
	/*const float */control=degPid.calc(deg);//+ddegPid.calc(imu.gyroZ.getDDeg());
	
	const float diffAbs=abs(getDegDiff());
	if(diffAbs<0.0){
		motorL.output(0.0);
		motorR.output(0.0);
	}else{
		motorL.output(control);
		motorR.output(control);
	}
	/*
	if(-0.9<control && control<0.9){
		controlSum+=control;
		/*const float controllSumLimit=15.0;
		if(controllSum>controllSumLimit){
			controllSum=0.0;
			targetDeg+=0.05;
		}else if(controllSum<-controllSumLimit){
			controllSum=0.0;
			targetDeg-=0.05;
		}*
		
		const float controlSumLimit=35.0;
		if(controlSum>controlSumLimit){
			controlSum=controlSumLimit;
		}else if(controlSum<-controlSumLimit){
			controlSum=-controlSumLimit;
		}
		targetDeg-=targetDegPid.calc(controlSum);
	}*/
	//targetDegAdd=-targetDegPid.calc(displacement);
	//targetDeg=targetDeg*targetDegBaseChangeMult+(targetDeg+targetDegAdd)*(1.0-targetDegBaseChangeMult);
	
	/*if(abs(displacement-displacementLast)>30.0){
		if(displacement>displacementLast){
			targetDeg+=0.05;
		}else{
			targetDeg-=0.05;
		}
		displacementLast=displacement;
	}*/
	
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
void calcDisplacement(){
	const int pulsePerRevolution=125*4;
	displacement=(80.0*3.1415926535*displacementEnc.read())/pulsePerRevolution;
}

void setMove(const float valL,const float valR){
	motorL.setBase(valL);
	motorR.setBase(valR);
}

void setUserAdd(const float add){//setUSERRRRRRRRRRR
	
	displacementAdd+=add*4.0;
	targetDegPid.set(displacementAdd);
	//targetDegUser=-add*0.2;
	
	/*if(-0.005<add && add<0.005){
		return;
	}else{
		targetDegUser=-add*0.5;
		targetDegPid.set(displacement);
	}*/
}
void resetGyroAndPid(){
	degPid.reset();
	
}
void printDeg(){
	using namespace rob;
	pc.printf("  TDPidRead:%8s",flt(targetDegPid.read()));
	pc.printf("  displacement:%8s targetDeg:%7s +Add:%7s deg:%7s",flt(displacement),flt(targetDeg),flt(targetDeg+targetDegAdd),flt(deg));
	
	pc.printf("\n");
}
float getDeg(){
	return deg;
}

void setupRun(){
	resetGyroAndPid();
	targetDegPid.set(0.0);
}
void loopRun(){
	pidAndOutput();
	if(base::isEmerg()){
		motorL.stop();
		motorR.stop();
	}
	degPid.setGain(degGainP,degGainI,degGainD);
	degPid.set(targetDeg+targetDegAdd+targetDegUser);
	targetDegPid.setGain(targetDegGainP,targetDegGainI,targetDegGainD);
	
}

}//namespace run
