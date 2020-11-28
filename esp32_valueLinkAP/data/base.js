let base=(function(){
	const _tomoshibiHost="10.114.51.50";
	const _dragonHost="10.114.51.51";
	
	let _myHostname=location.hostname;
	if(_myHostname==""){
		_myHostname=_tomoshibiHost;
	}
	
	let _name="";
	let _otherHostname="";
	let _otherName=""
	if(_myHostname==_dragonHost){
		_name="Dragon";
		_otherHostname=_tomoshibiHost;
		_otherName="Tomoshibi";
	}else if(_myHostname==_tomoshibiHost){
		_name="Tomoshibi";
		_otherHostname=_tomoshibiHost;
		_otherName="Dragon";
	}else{
		_name="Unknown Host";
	}
	
	let global={
		hostname:_myHostname,
		myName:_name,
		otherHostname:_otherHostname,
		otherURL:"http://"+_otherHostname+"/",
		otherName:_otherName
	};
	return global;
})();


class logPrinter{
	constructor(){
		this.list=[]
		for(let i=0;i<16;i++){
			this.list.push(".");
		}
		this.elem=document.getElementById("logField");
	}
	addLine(str){
		this.list.shift();
		this.list.push(str);
	}
	add(str){
		const maxPerLine=40;
		while(true){
			if(str.length>maxPerLine){
				this.addLine(str.slice(0,maxPerLine));
			}else{
				break;
			}
			str=str.slice(maxPerLine);
		}
		this.addLine(str);
		this.elem.innerHTML="";
		for(const i of this.list){
			this.elem.innerHTML+=i+"<br>";
		}
	}
}
let logger=new logPrinter();

function log(str){
	logger.add(str);
}


function turnFullScreenOnOff(){
	let elem=document.getElementById("fullScreen");
	if(document.fullscreenElement==null){
		document.body.requestFullscreen();
		elem.innerHTML="Stop full screen.";
	}else{
		document.exitFullscreen();
		elem.innerHTML="Make it full screen.";
	}
}



(function(){
	//マシンに即した設定
	let gotoOtherElem=document.getElementById("gotoOther");
	gotoOtherElem.setAttribute("href",base.otherURL);
	gotoOtherElem.innerHTML+=base.otherName;
	
	let viewMainElem=document.getElementById("viewMain");
	let name=base.myName;
	log(name);
	if(name=="Dragon"){
		viewMainElem.style.backgroundColor="#333";
	}else if(name=="Tomoshibi"){
		viewMainElem.style.backgroundColor="#fff";
	}else{
		viewMainElem.style.backgroundColor="green";
	}
	//log(base.otherURL+"asdasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");
})();