#ifndef BASE_HPP_INCLUDE_GUARD
#define BASE_HPP_INCLUDE_GUARD


#include "2020roboOneBoardLib_setting.hpp"

namespace base{
	extern bool isEmergVal;
	
	inline bool setEmerg(const bool val){
		return isEmergVal=val;
	}
	inline bool isEmerg(){
		return isEmergVal;
	}
	inline bool turnEmerg(){
		return isEmergVal=!isEmergVal;
	}
	static const float TARGET_DEG_INIT=33.7;
}


namespace deg{
static const int CALC_DEG_INTERVAL=300;//us
static const int CALC_DEG_IMU_INTERVAL=700;//us
static const int CALC_DEG_IMU_OFFSET_INTERVAL=50000;//us

extern rob::a_imu03a &imu;
extern rob::aRotaryEncoder &rorycon;

extern float max;

//extern float deg;
extern float rawDegRorycon,degRorycon,degAccel,degGyro;
inline float get(){
	return degRorycon*0.9+degGyro*0.1;
}

void setupDeg();
void loopDeg();
}

namespace axisX{
static const int CALC_VALX_INTERVAL=300;//us

extern float valX;
inline float get(){
	return valX;
}

void loopAxisX();
}

#endif