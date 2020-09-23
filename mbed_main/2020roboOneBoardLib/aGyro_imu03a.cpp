#include "aGyro_imu03a.hpp"

namespace rob{
//int moniva;

a_imu03a::a_imu03a(
	PinName mosi,
	PinName miso,
	PinName sclk,
	PinName csPin,
	bool isForward
):	
	sp(mosi,miso,sclk),
	cs(csPin),
	whoamiCont(0)
{
	//wait_ms(23);
	//pc.printf("jey\n");
	sp.frequency(1000000);
	sp.format(8,3);

	comE();
	wait_ms(23);
	resetModule();
	
	resetDeg();
	
	isForwardVal=isForward;
}

void a_imu03a::resetModule(){
	int val;
	
	/*comS();
	sp.write(0x80|0x0f);
	val=sp.write(0x00);
	//pc.printf("who am i return 0x%2X\n",val);
	comE();*/
	
	comS();
	sp.write(0x80|0x12);
	val=sp.write(0x00);
	comE();
	//pc.printf("0x12 val is 0x%2X\n",val);
	
	//CTRL3_C に値を書き込んでModuleをリセットする
	//set CTRL3_C register
	comS();
	sp.write(0x12);
	sp.write(val|0x1);
	comE();
	
	wait_ms(23);
	
	//CTRL2_G に設定値を書きこんでgyroを起動
	//設定値は1.66kHz 500dps
	comS();
	sp.write(0x11);
	sp.write(0b10000100);
	comE();
	
	wait_ms(83);
	
	offsetVal=getOffset();
	
	isNormalVal=true;
}

void a_imu03a::resetDeg(){
	deg=0.0;
	tc.detach();
}

void a_imu03a::startDeg(){
	if(isForwardVal){
		tc.attach_us(callback(this,&a_imu03a::sumDdegP),deltaT);
	}else{
		tc.attach_us(callback(this,&a_imu03a::sumDdegM),deltaT);
	}
}

void a_imu03a::stopDeg(){
	tc.detach();
}

void a_imu03a::sumDdegP(){
	deg+=((double)deltaT/1000000.0)*getDdeg();
}
void a_imu03a::sumDdegM(){
	deg-=((double)deltaT/1000000.0)*getDdeg();
}
	

double a_imu03a::getOffset(){
	const int N=1000;
	long sum=0;
	for(int i=0;i<N;i++){
		sum+=getRawVal();
		wait_us(625);
	}
	return sum/N;
}


double a_imu03a::getDdeg(){
	/*const int N=10;
	double ddeg=0.0;
	for(int i=0;i<N;i++){
		ddeg+=getRawDdeg()-offsetDdeg;
	}
	return ddeg/(double)N;*/
	return rawValToDdeg(getRawVal()-offsetVal);
}
/*double a_imu03a::getRawDdeg(){
	//z軸のみ
	int16_t rval;
	
	comS();
	sp.write(0x80|0x26);
	rval=sp.write(0x00);
	comE();
	
	comS();
	sp.write(0x80|0x27);
	rval|=sp.write(0x00)<<8;
	comE();
	
	moniva=0xffff&rval;
	
	return (rval*125.0)/(double)0x7fff;
}*/
int16_t a_imu03a::getRawVal(){
	int16_t rval;
	
	comS();
	sp.write(0x80|0x26);
	rval=sp.write(0x00);
	comE();
	
	comS();
	sp.write(0x80|0x27);
	rval|=sp.write(0x00)<<8;
	comE();
	
	//moniva=0xffff&rval;
	
	if(whoamiCont>100){
		isNormalVal=isNormalChecker();
	}
	whoamiCont++;
	
	return rval;
}
double a_imu03a::rawValToDdeg(int16_t val){
	return (val*500.0)/(double)0x7fff;
}

bool a_imu03a::isNormalChecker(){
	int val;
	comS();
	sp.write(0x80|0x0f);
	val=sp.write(0x00);
	//pc.printf("who am i return 0x%2X\n",val);
	comE();
	return (val==0x69);
}

}