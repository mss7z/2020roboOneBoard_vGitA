#ifndef BUTTON_VL_INCLUDE_GUARD
#define BUTTON_VL_INCLUDE_GUARD

#include "useful.hpp"
#include "mbed.h"

namespace vl{


//ひどすぎる
//ごみのような設計
class crossBtnVL
{
	private:
	Callback<void(char)> callbackFunc;
	
	public:
	static const char ID='+';
	static const char JSON_STR[];
	crossBtnVL();
	void attach(Callback<void(char)>);
	void listener(char);
	const char* getJsonStr();
};



}
#endif