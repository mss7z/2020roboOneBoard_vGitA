#include "aRotaryEncoder_v2020.hpp"

namespace rob{
namespace aRotaryEncoder_v2020_internal{
	/*
	PinMode list
	PullUp / PullDown / PullNone
	*/
	aRotaryEncoder::aRotaryEncoder(PinName AphsPin,PinName BphsPin,PinMode mode,bool isForward):
		Aphs(AphsPin),//ここで初期化
		BphsInter(BphsPin)
	{
		Aphs.mode(mode);
		BphsInter.mode(mode);
		if(isForward){
			BphsInter.rise(callback(this, &aRotaryEncoder::BphsRiseProcF));
		}else{
			BphsInter.fall(callback(this, &aRotaryEncoder::BphsRiseProcF));
		}
		val=0;
		diff=0;
	}

	void aRotaryEncoder::BphsRiseProcF(){
		if(Aphs)
			val++;
		else
			val--;
		return;
	}

}//end of aRotaryEncoder_internal
}