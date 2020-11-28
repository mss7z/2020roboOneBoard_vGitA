#include "ajustVL.hpp"

namespace vl{


ajustVLbaseBase::ajustVLbaseBase(const char *namea){
	strcpy(name,namea);
}
void ajustVLbaseBase::publishNameTo(addValAndStrInterface *doc){
	doc->addStr(smallIDToBigID(smallID),name);
}
void ajustVLbaseBase::publishAllTo(addValAndStrInterface *doc){
	publishNameTo(doc);
	publishValTo(doc);
}

ajustVLmanager::ajustVLmanager():listTailIndex(0){}
char ajustVLmanager::entryAndGetSmallID(ajustVLbaseBase *objP){
	if(listTailIndex>=LIST_LEN)return '\0';
	list[listTailIndex]=objP;
	return indexToSmallID(listTailIndex++);
}
char *ajustVLmanager::getAllJsonStr(){
	jsonGen.initialize();
	for(int i=0;i<listTailIndex;i++){
		list[i]->publishAllTo(&jsonGen);
	}
	jsonGen.finalize();
	return jsonGen.getStr();
}
char *ajustVLmanager::getStdJsonStr(){
	jsonGen.initialize();
	for(int i=0;i<listTailIndex;i++){
		list[i]->publishStdTo(&jsonGen);
	}
	jsonGen.finalize();
	return jsonGen.getStr();
}
ajustVLbaseBase* ajustVLmanager::refBySmallID(const char c){
	const int index=smallIDToIndex(c);
	if(!(index<listTailIndex)){
		return NULL;
	}
	return list[index];
}

ajustVLbase::ajustVLbase(ajustVLmanager &mother,const char *namea):
	ajustVLbaseBase(namea)
{
	smallID=mother.entryAndGetSmallID(this);
}

ajustFloatVL::ajustFloatVL(ajustVLmanager &mother,const char *namea,float *vala,const float stepa):
	ajustVLbase(mother,namea),val(vala),step(stepa)
{}
void ajustFloatVL::publishValTo(addValAndStrInterface *doc){
	doc->addVal(smallID,*val);
}
void ajustFloatVL::publishStdTo(addValAndStrInterface *doc){
	if(!isSame(*val)){
		publishValTo(doc);
	}
}
void ajustFloatVL::ajust(const float mult){
	*val+=step*mult;
	//pc.printf("val:%f mult:%f\n",*val,mult);
}

}
	