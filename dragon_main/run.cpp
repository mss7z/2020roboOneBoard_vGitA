#include "run.hpp"

namespace run{
motor motorL(rob::tb6643kq_md3,-1.0);
motor motorR(rob::tb6643kq_md4,1.0);

//float degGainP=0.00471;float degGainI=0.01077;float degGainD=0.00005345;//0A16 300us 50times
//float degGainP=0.00391;float degGainI=0.01077;float degGainD=0.00004225;//0A17 testrun 1719 300us 50times
//float degGainP=0.00400;float degGainI=0.00847;float degGainD=0.00005345;//0A17 testrun 1719 300us 50times
//float degGainP=0.00969;float degGainI=0.01027;float degGainD=0.00006535;//0B22 testrun 1719 300us 50times
float degGainP=0.00669;float degGainI=0.01347;float degGainD=0.00005084;//0B22 125mm tiaya


//float degGainP=0.001812;float degGainI=0.000013;float degGainD=0.00;
rob::aPid<float> degPid(degGainP,degGainI,degGainD,CONTROL_CYCLE_TIME_SEC,PID_OPERATION_MAX,PID_OPERATION_MIN);

//float targetDegGainP=0.00000575;float targetDegGainI=0.00000160;float targetDegGainD=0.00000041;//0a18 good 0919
//float targetDegGainP=0.00000575;float targetDegGainI=0.00000129;float targetDegGainD=0.00000041;//0a18 good 0919
//float targetDegGainP=0.00000675;float targetDegGainI=0.00000178;float targetDegGainD=0.00000048;//0a18 good 0919
//float targetDegGainP=0.00000675;float targetDegGainI=0.00001877;float targetDegGainD=0.00000050;//0b15 good
//float targetDegGainP=0.00000675;float targetDegGainI=0.00001877;float targetDegGainD=0.00000050;//0b15 good
float targetDegGainP=0.00001375;float targetDegGainI=0.00003647;float targetDegGainD=0.00000037;//0b22 125mm taiya


//float targetDegGainP=0.00000575;float targetDegGainI=0.00000072;float targetDegGainD=0.00000264;//0A07最初
rob::aPid<float> targetDegPid(targetDegGainP,targetDegGainI,targetDegGainD,CONTROL_CYCLE_TIME_SEC,3.0,-3.0);

float realOutputTimeVal=0.0;

float userControll=0.0;
float control=0.0;
float controlSum=0.0;

float targetDeg=base::TARGET_DEG_INIT;
float targetDegAdd=0.0;
float targetDegUser=0.0;

float targetDegBaseChangeMult=0.999;

float displacementAdd=0.0;


//内部
float getDegDiff();
void pidAndOutput();
void calcAndSetRotate();
void calcDisplacement();


float getDegDiff(){
	return degPid.read()-deg::get();
}
void pidAndOutput(){
	static rob::regularC_us outputTime(CONTROL_CYCLE_TIME);
	
	static rob::regularC_ms calcTargetDegTime(100);
	
	
	if(base::isEmerg()){
		return;
	}
	if(calcTargetDegTime){
		targetDegAdd=-targetDegPid.calc(axisX::get());
		targetDeg=targetDeg*targetDegBaseChangeMult+(targetDeg+targetDegAdd)*(1.0-targetDegBaseChangeMult);
	}
	
	if(!outputTime){
		return;
	}
	
	/*const float */control=degPid.calc(deg::get());//+ddegPid.calc(imu.gyroZ.getDDeg());
	
	const float diffAbs=abs(getDegDiff());
	if(diffAbs<0.0){
		motorL.output(0.0);
		motorR.output(0.0);
	}else{
		motorL.output(control);
		motorR.output(control);
	}
}

void setMove(const float valL,const float valR){
	pc.printf("L:%f R:%f",valL,valR);
	motorL.setBase(valL);
	motorR.setBase(valR);
}

void setUserAdd(const float add){//setUSERRRRRRRRRRR
	static float displacementAddFilter=0.0;
	displacementAdd+=add*10.0;//4.0
	displacementAddFilter=0.3*displacementAdd+0.7*displacementAddFilter;
	targetDegPid.set(displacementAddFilter);
}

float rotateSumaho=0.0,rotateController=0.0;
void setUserRotateSumaho(const float val){
	//複数台のスマホでやった時やべえ
	rotateSumaho=val*0.36;
	calcAndSetRotate();
}
void setUserRotateController(const float val){
	rotateController=val*0.35;
	calcAndSetRotate();
}
void calcAndSetRotate(){
	const float rot=(rotateSumaho+rotateController);
	setMove(rot,-rot);
}
void resetGyroAndPid(){
	degPid.reset();
	
}
void printDeg(){
	using namespace rob;
	return;
	pc.printf(" disp(tag:%5d now:%5d)",(int)targetDegPid.read(),(int)axisX::get());
	pc.printf(" targetDeg:%7s +Add:%7s deg:%7s",flt(targetDeg),flt(targetDeg+targetDegAdd),flt(deg::get()));
	pc.printf(" gyro:%7s acel:%7s rory:%7s",flt(deg::degGyro),flt(deg::degAccel),flt(deg::degRorycon));
	pc.printf(" diff:%6s rpm:%6s",flt(deg::get()-degPid.read()),flt(deg::max));
	
	pc.printf("\n");
}

bool isGoodDeg(){
	static const float PM=0.3;
	return PM>abs(getDegDiff());
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
