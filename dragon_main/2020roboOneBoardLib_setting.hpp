#ifndef M2020ROBOONEBOARDLIB_INCLUDE_GUARD
#define M2020ROBOONEBOARDLIB_INCLUDE_GUARD

#include "2020roboOneBoardLib/lib.hpp"

//#define ENABLE_tb6643kq_md1
//#define ENABLE_tb6643kq_md2
#define ENABLE_tb6643kq_md3
#define ENABLE_tb6643kq_md4
#define ENABLE_rotaryEncoder1
#define ENABLE_rotaryEncoder2
#define ENABLE_imu03a
#define ENABLE_xbeeCore
#define VAL_xbeeCore_serialSpeed 115200
#define VAL_xbeeCore_nodeNum 2


namespace rob{
#ifdef ENABLE_tb6643kq_md1
extern aTB6643KQ tb6643kq_md1;
#endif

#ifdef ENABLE_tb6643kq_md2
extern aTB6643KQ tb6643kq_md2;
#endif

#ifdef ENABLE_tb6643kq_md3
extern aTB6643KQ tb6643kq_md3;
#endif

#ifdef ENABLE_tb6643kq_md4
extern aTB6643KQ tb6643kq_md4;
#endif

#ifdef ENABLE_rotaryEncoder1
//aRotaryEncoder rotaryEncoder1(PA_11,PA_12,PullDown);
extern aRotaryEncoder rotaryEncoder1;
#endif

#ifdef ENABLE_rotaryEncoder2
extern aRotaryEncoder rotaryEncoder2;
#endif

#ifdef ENABLE_imu03a
//mosi miso slck csPin
extern a_imu03a imu03a;
#endif

#ifdef ENABLE_xbeeCore
//tx rx 
extern aXbeeCoreCallback<VAL_xbeeCore_nodeNum> xbeeCore;
#endif

}

#endif
