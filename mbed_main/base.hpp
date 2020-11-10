#ifndef BASE_HPP_INCLUDE_GUARD
#define BASE_HPP_INCLUDE_GUARD


#include "2020roboOneBoardLib/lib.hpp"

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
static const int CALC_DEG_IMU_OFFSET_INTERVAL=3000;//us

extern rob::a_imu03a &imu;
extern rob::aRotaryEncoder &rorycon;

extern float max;

//extern float deg;
extern float rawDegRorycon,degRorycon,degImu,accelDeg,gyroDeg;
inline float get(){
	return degRorycon*0.9+degImu*0.1;
}

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