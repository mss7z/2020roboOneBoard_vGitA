let socket=new WebSocket("ws://jaxson.local:81/");
let statusElem=document.getElementById("status");

class logPrinter{
	constructor(){
		this.list=[]
		for(let i=0;i<100;i++){
			this.list.push(".");
		}
		this.elem=document.getElementById("logField");
	}
	add(str){
		this.list.shift();
		this.list.push(str);
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
function send(msg){
	socket.send(msg);
	log("[send] data:"+msg);
}

class value{
	addSingleBtn(sendVal,classVal,innerVal){
		const selfID=this.smallID;
		let down=document.createElement("div");
		down.setAttribute("class",classVal);
		down.innerHTML=innerVal;
		down.onclick=function(){
			send(`\{\"${selfID}\":${sendVal}\}`);
		}
		let listener=function(endEvent){
			const intervalID=setInterval(function(){
				send(`\{\"${selfID}\":${sendVal}\}`);
			},300);
			const endEventListener=function(){
				return function f(){
					down.removeEventListener(endEvent,f,false);
					clearInterval(intervalID);
				}
			}
			down.addEventListener(endEvent,endEventListener());
		}
		down.addEventListener("mousedown",function(){
			listener("mouseup");
		});
		down.addEventListener("touchstart",function(){
			listener("touchend");
		});
		return down;
	}
	createMyDiv(){
		this.selfMother=document.createElement("div");
		this.selfMother.setAttribute("id","smallID-"+this.smallID);
		
		this.nameField=document.createElement("p");
		this.nameField.setAttribute("class","name");
		this.selfMother.appendChild(this.nameField);
		
		this.valField=document.createElement("p");
		this.valField.setAttribute("class","value");
		this.selfMother.appendChild(this.valField);
		
		let mother=document.getElementById("valueField");
		mother.appendChild(this.selfMother);
	}
	addBtnToMyDiv(){
		const selfSmallID=this.smallID;
		
		this.down=this.addSingleBtn("1.0","downBtn btn","-");
		this.selfMother.insertBefore(this.down,this.selfMother.firstChild);
		
		this.up=this.addSingleBtn("1.0","upBtn btn","+");
		this.selfMother.insertBefore(this.up,null);
	}
		
	insertName(){
		this.nameField.innerHTML=this.name;
	}
	insertVal(){
		this.valField.innerHTML=this.val;
	}
	
	setVal(vala){
		this.val=vala;
		this.insertVal();
	}
	setName(namea){
		this.name=namea;
		this.insertName();
	}
	
	constructor(smallIDa){
		this.smallID=smallIDa;
		this.name="NULL NULL";
		this.createMyDiv();
		this.insertName();
		
		this.val="1.0";
		this.insertVal();
		
		this.addBtnToMyDiv();
	}
	
}
/*
let testVal=new value("x");
testVal.setVal(6.99);
testVal.setName("testValName");*/

class crossBtn{
	addSingleBtn(sendVal,classVal,innerVal){
		const selfID=this.id;
		let down=document.createElement("div");
		down.setAttribute("class",classVal);
		down.innerHTML=innerVal;
		const sendValFunc=function(){
			send(`\{\"${selfID}\":\"${sendVal}\"\}`);
		}
		let listener=function(endEvent){
			const intervalID=setInterval(sendValFunc,100);
			sendValFunc();
			const endEventListener=function(){
				return function f(){
					down.removeEventListener(endEvent,f,false);
					clearInterval(intervalID);
					send(`\{\"${selfID}\":\"0\"\}`);
				}
			}
			down.addEventListener(endEvent,endEventListener());
		}
		down.addEventListener("mousedown",function(){
			listener("mouseup");
		});
		down.addEventListener("touchstart",function(){
			listener("touchend");
		});
		return down;
	}
		
	createMyDiv(){
		this.selfMother=document.createElement("div");
		this.selfMother.setAttribute("id","crossBtn-"+this.id);
		
		this.crossMother=document.createElement("div");
		this.crossMother.setAttribute("class","crossBtn");
		
		const crossMotherAbs=document.createElement("div");
		crossMotherAbs.setAttribute("class","crossBtnAbs");
		crossMotherAbs.appendChild(this.crossMother);
		
		this.down=this.addSingleBtn("D","downCrossBtn btn","D");
		this.crossMother.appendChild(this.down);
		
		this.up=this.addSingleBtn("U","upCrossBtn btn","U");
		this.crossMother.appendChild(this.up);
		
		this.left=this.addSingleBtn("L","leftCrossBtn btn","L");
		this.crossMother.appendChild(this.left);
		
		this.right=this.addSingleBtn("R","rightCrossBtn btn","R");
		this.crossMother.appendChild(this.right);
		
		this.selfMother.appendChild(crossMotherAbs);
		
		let mother=document.getElementById("btnField");
		mother.appendChild(this.selfMother);
	}
	constructor(id){
		this.id=id;
		this.createMyDiv();
	}
}

class valueManager{
	isID(c){
		return c.length==1;
	}
	isBigID(c){
		const co=c.charCodeAt();
		return 'A'.charCodeAt()<=co && co<='Z'.charCodeAt();
	}
	bigIDtoSmallID(c){
		const str=c.toLowerCase();
		return str[0];
	}
	procID(id,vals){
		if(id[0]=='+'){
			if(!(id in this.list)){
				this.list[id]=new crossBtn(id);
			}
			return true;
		}
		return false;
	}
	procValueID(id,vals){
		const smallID=this.bigIDtoSmallID(id);
		if(!(smallID in this.list)){
			this.list[smallID]=(new value(smallID));
		}
		if(this.isBigID(id)){
			this.list[smallID].setName(vals);
		}else{
			this.list[smallID].setVal(vals);
		}
	}
	procJson(json){
		for(const i in json){
			if(this.isID(i)){
				const id=i[0];
				if(this.procID(id,json[i])){
					continue;
				}else{
					this.procValueID(id,json[i]);
				}
			}
		}
	}
	constructor(){
		this.list={};
	}
}
let manager=new valueManager();
let json=JSON.parse("{\"A\":\"name\",\"a\":3.145,\"+\":\"\"}");
manager.procJson(json);
	

socket.onopen = function(e) {
	log("[open] Connection established");
	//log("Sending to server");
	//socket.send("My name is John");
	statusElem.style.backgroundColor="green";
	statusElem.innerHTML="Connected!";
};

socket.onmessage = function(event) {
	log(`[message] Data received from server: ${event.data}`);
	let receive=JSON.parse(event.data);
	
	manager.procJson(receive);
};

socket.onclose = function(event) {
	if (event.wasClean) {
		log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
	} else {
		// e.g. サーバのプロセスが停止、あるいはネットワークダウン
		// この場合、event.code は通常 1006 になります
		log('[close] Connection died');
	}
	statusElem.style.backgroundColor="red";
	statusElem.innerHTML="Disconnected";
};

socket.onerror = function(error) {
	log(`[error] ${error.message}`);
};
/*
function settingHP(){
	var fullScreenOnElem=document.getElementById("fullScreenOn");
	fullScreenOnElem.addEventListener("click",function (){
		document.body.requestFullscreen();
	},false);
	//https://developers.google.com/web/fundamentals/native-hardware/fullscreen?hl=ja
}*/
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