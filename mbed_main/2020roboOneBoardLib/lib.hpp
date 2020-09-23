#ifndef V2020ROBOONEBOARD_LIB_HPP_INCLUDE_GUARD
#define V2020ROBOONEBOARD_LIB_HPP_INCLUDE_GUARD

#include "useful.hpp"
#include "aTB6643KQ.hpp"
#include "aRotaryEncoder_v2020.hpp"
#include "aGyro_imu03a.hpp"
#include "aXbeeCore.hpp"
#include "aXbee.hpp"
#include "aPid.hpp"

namespace rob{


#ifdef ENABLE_tb6643kq_md1
aTB6643KQ tb6643kq_md1(PB_4,PC_8);
#endif

#ifdef ENABLE_tb6643kq_md2
aTB6643KQ tb6643kq_md2(PC_9,PB_8);
#endif

#ifdef ENABLE_tb6643kq_md3
aTB6643KQ tb6643kq_md3(PB_3,PA_10);
#endif

#ifdef ENABLE_tb6643kq_md4
aTB6643KQ tb6643kq_md4(PB_10,PB_5);
#endif

#ifdef ENABLE_rotaryEncoder1
aRotaryEncoder rotaryEncoder1(PA_11,PA12,PullDown);
#endif

#ifdef ENABLE_rotaryEncoder2
aRotaryEncoder rotaryEncoder2(PA_8,PA_9,PullDown);
#endif

#ifdef ENABLE_imu03a
//mosi miso slck csPin isForward
a_imu03a imu03a(PC_12,PC_11,PC_10,PB_1,false);
#endif

#ifdef ENABLE_xbeeCore
//tx rx 
aXbeeCoreSingleCallback xbeeCore(PC_6,PC_7,VAL_xbeeCore_serialSpeed);
#endif

xbee64bitAddress ab(0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36);
aXbeeCom adb(xbeeCore,xbee64bitAddress(0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36));

}

#endif