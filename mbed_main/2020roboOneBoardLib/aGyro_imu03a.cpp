#include "aGyro_imu03a.hpp"

namespace rob{

namespace a_imu03a_internal{
	
imu03aSPI::imu03aSPI(
	PinName mosi,
	PinName miso,
	PinName sclk,
	PinName csPin
):
	sp(mosi,miso,sclk),
	cs(csPin)
{
	//通信速度1MHz
	sp.frequency(1000000);
	//ビット長8bit, mode 立下り先行シフト先行
	sp.format(8,3);
	//https://os.mbed.com/users/okini3939/notebook/SPI_jp/
	
	end();
	//wait_ms(23);
}

imu03aSetting::imu03aSetting(imu03aSPI &c):
	com(c)
{}
void imu03aSetting::resetModule(){
	int val;
	
	com.start();
	com.write(0x80|0x12);
	val=com.write(0x00);
	com.end();
	//pc.printf("0x12 val is 0x%2X\n",val);
	
	//CTRL3_C に値を書き込んでModuleをリセットする
	//set CTRL3_C register
	com.start();
	com.write(0x12);
	com.write(val|0x1);
	com.end();
	
	wait_ms(23);
	
	//CTRL1_XL に設定値を書きこんでacceleration sensorを起動
	//設定値は1.66kHz +-2g
	//Anti-aliasing filter bandwidth(これより高い周波数成分を消すlowpass filter) 50Hz
	com.start();
	com.write(0x10);
	com.write(0b10000011);
	com.end();
	
	com.start();
	com.write(0x13);
	com.write(0b10000000);
	com.end();
	
	//CTRL2_G に設定値を書きこんでgyroを起動
	//設定値は1.66kHz 500dps
	com.start();
	com.write(0x11);
	com.write(0b10000100);
	com.end();
	
	//wait_ms(83);
	
}

bool imu03aSetting::isNormal(){
	//who am i レジスターが0x69を返すかどうか
	int val;
	com.start();
	com.write(0x80|0x0f);
	val=com.write(0x00);
	//pc.printf("who am i return 0x%2X\n",val);
	com.end();
	return (val==0x69);
}

imu03aGyroAndAccelBase::imu03aGyroAndAccelBase(imu03aSPI& c,const uint8_t l,const uint8_t h):
	LbitReg(l),HbitReg(h),com(c)
{}
int16_t imu03aGyroAndAccelBase::getRawVal(){
	int16_t rval;
	
	com.start();
	com.write(0x80|LbitReg);
	rval=com.write(0x00);
	com.end();
	
	com.start();
	com.write(0x80|HbitReg);
	rval|=com.write(0x00)<<8;
	com.end();
	
	return rval;
}

int16_t imu03aGyro::getOffsetRaw(){
	const int N=1000;//1000
	long sum=0;
	for(int i=0;i<N;i++){
		sum+=imu03aGyroAndAccelBase::getRawVal();
		wait_us(625);
	}
	return sum/N;
}

float imu03aGyro::rawVal2DDeg(const int16_t val){
	//定数についてはimu03aSetting::resetModuleを参照のこと
	return (val*500.0)/(float)0x7fff;
}
int16_t imu03aGyro::ddegToRawVal(const float val){
	return (int16_t)((val/500.0)*(float)0x7fff);
}

void imu03aGyro::sumDdegP(){
	deg+=((float)deltaT/1000000.0)*getDDeg();
}
float imu03aGyro::getDDeg(){
	return rawVal2DDeg(imu03aGyroAndAccelBase::getRawVal()-offsetRawVal);
}

void imu03aGyro::resetOffset(){
	offsetRawVal=getOffsetRaw();
}
void imu03aGyro::calcOffsetByTrueDdeg(const float ddeg,const float mult){
	const float estimated=imu03aGyroAndAccelBase::getRawVal()-ddegToRawVal(ddeg);
	offsetRawVal=(int16_t)(mult*estimated+(1.0-mult)*offsetRawVal);
}
	
/*void imu03aGyro::resetDeg(){
	deg=0.0;
	tc.detach();
}
void imu03aGyro::startDeg(){
	tc.attach_us(callback(this,&imu03aGyro::sumDdegP),deltaT);
}
void imu03aGyro::stopDeg(){
	tc.detach();
}*/

float imu03aAccel::rawVal2G(const int16_t val){
	//定数についてはimu03aSetting::resetModuleを参照のこと
	return (val*2.0)/(float)0x7fff;
}
float imu03aAccel::getG(){
	return rawVal2G(imu03aGyroAndAccelBase::getRawVal());
}
float imu03aAccel::getMperS2(){
	//1G=9.80665m/s^2
	return 9.80665*getG();
}

a_imu03a::a_imu03a(
	PinName mosi,
	PinName miso,
	PinName sclk,
	PinName csPin
):	
	com(mosi,miso,sclk,csPin),
	setting(com),
	//レジスターの値はデータシートを参照のこと
	gyroX(com,0x22,0x23),
	gyroY(com,0x24,0x25),
	gyroZ(com,0x26,0x27),
	accelX(com,0x28,0x29),
	accelY(com,0x2A,0x2B),
	accelZ(com,0x2C,0x2D)
{
	resetModule();
}
void a_imu03a::resetModule(){
	setting.resetModule();
	
	gyroX.resetOffset();
	gyroY.resetOffset();
	gyroZ.resetOffset();
}

namespace old{

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
}//namespace old

}//namespace a_imu03a_internal
}//namespace rob