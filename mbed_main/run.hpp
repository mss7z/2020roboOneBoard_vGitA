#ifndef RUN_HPP_INCLUDE_GUARD
#define RUN_HPP_INCLUDE_GUARD

#include "base.hpp"

namespace run{


class motor{
	private:
	rob::aTB6643KQ &mtInst;
	float baseOutput;
	const float constMult;
	public:
	motor(rob::aTB6643KQ &mt,const float cm):mtInst(mt),baseOutput(0.0),constMult(cm){}
	void setBase(const float val){baseOutput=val;}
	void output(const float val){mtInst=constMult*(val+baseOutput);}
	void stop(){mtInst=0.0;}
};

const int CONTROL_CYCLE_TIME=5000;//us
const float CONTROL_CYCLE_TIME_SEC=CONTROL_CYCLE_TIME/1000000.0;//sec
const float PID_OPERATION_MAX=1.0,PID_OPERATION_MIN=-1.0;

extern float degGainP,degGainI,degGainD;

extern float targetDegGainP,targetDegGainI,targetDegGainD;

extern float control,controlSum;
extern float targetDeg;

extern float targetDegBaseChangeMult;


//外部
void setMove(const float valL,const float valR);
void setUserAdd(const float);
void setUserRotateSumaho(const float);
void setUserRotateController(const float);
void resetGyroAndPid();
void printDeg();
bool isGoodDeg();

void setupRun();
void loopRun();

}//namespace run

#endif