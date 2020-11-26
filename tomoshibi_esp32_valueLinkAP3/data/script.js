let socket=new WebSocket("ws://jaxson.local:81/");
let statusElem=document.getElementById("status");

class logPrinter{
	constructor(){
		this.list=[]
		for(let i=0;i<10;i++){
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
		
		this.down=document.createElement("div");
		this.down.setAttribute("class","downBtn btn");
		this.down.innerHTML="-";
		this.down.onclick=function(){
			send(`\{\"${selfSmallID}\":-1.0\}`);
		}
		this.selfMother.insertBefore(this.down,this.selfMother.firstChild);
		
		this.up=document.createElement("div");
		this.up.setAttribute("class","upBtn btn");
		this.up.innerHTML="+";
		this.up.onclick=function(){
			send(`\{\"${selfSmallID}\":1.0\}`);
		}
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
let testVal=new value("x");
testVal.setVal(6.99);
testVal.setName("testValName");
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
	procJson(json){
		for(const i in json){
			if(this.isID(i)){
				const id=i[0];
				const smallID=this.bigIDtoSmallID(id);
				if(!(smallID in this.list)){
					this.list[smallID]=(new value(smallID));
				}
				if(this.isBigID(i)){
					this.list[smallID].setName(json[i]);
				}else{
					this.list[smallID].setVal(json[i]);
				}
			}
		}
	}
	constructor(){
		this.list={};
	}
}
let manager=new valueManager();
let json=JSON.parse("{\"A\":\"name\",\"a\":3.145}");
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