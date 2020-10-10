#ifndef BASE_HPP_INCLUDE_GUARD
#define BASE_HPP_INCLUDE_GUARD


#include "2020roboOneBoardLib/lib.hpp"
/*
#define VAL_xbeeCore_serialSpeed 38400
//ENABLE_tb6643kq_md1;
//ENABLE_tb6643kq_md2;
ENABLE_tb6643kq_md3;
ENABLE_tb6643kq_md4;
ENABLE_rotaryEncoder1;
ENABLE_rotaryEncoder2;
ENABLE_imu03a;
ENABLE_xbeeCore;*/

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
	
	//const float TARGET_DEG_INIT=34.628;
	//const float TARGET_DEG_INIT=32.676;
	///const float TARGET_DEG_INIT=30.876;
	static const float TARGET_DEG_INIT=35.4;
}

#endif