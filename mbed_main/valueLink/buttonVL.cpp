#include "buttonVL.hpp"


namespace vl{

crossBtnVL::crossBtnVL()
{
	
}


const char crossBtnVL::JSON_STR[]="{\"+\":\"\"}";

void crossBtnVL::attach(Callback<void(char)> val){
	callbackFunc=val;
	return;
}
void crossBtnVL::listener(char c){
	//pc.printf("cross btn listener called!");
	callbackFunc(c);
}

const char* crossBtnVL::getJsonStr(){
	return JSON_STR;
}

}