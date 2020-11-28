#include "valueTalkVL.hpp"

namespace vl{

void valTalkerMbedSerial::callbackFunc(){
	buf[(tail++)&BUF_MASK]=(uint8_t)srl.getc();
}

}