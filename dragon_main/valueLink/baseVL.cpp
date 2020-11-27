#include "baseVL.hpp"

namespace vl{

void simpleJsonGenerator::initialize(){
	*buf='{';
	tail=buf+1;
}

void simpleJsonGenerator::addVal(const char *key,const float val){
	tail+=sprintf(tail,"\"%s\":%.9f,",key,val);
}
void simpleJsonGenerator::addStr(const char *key,const char *str){
	tail+=sprintf(tail,"\"%s\":\"%s\",",key,str);
}
void simpleJsonGenerator::addVal(const char key,const float val){
	tail+=sprintf(tail,"\"%c\":%.9f,",key,val);
}
void simpleJsonGenerator::addStr(const char key,const char *str){
	tail+=sprintf(tail,"\"%c\":\"%s\",",key,str);
}
void simpleJsonGenerator::finalize(){
	if(*(tail-1)==',')tail--;
	*tail='}';tail++;
	*tail='\0';
}

}