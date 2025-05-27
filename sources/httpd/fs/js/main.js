var txt = {
	"dev_name":"<!--#system_device_name-->",
	<!--#system_network-->,
	"serial_no":"<!--#system_serial_num-->",
	"ver_fw":"<!--#system_fw_ver-->",
	"ver_hw":"<!--#system_hw_ver-->",
	"uptime":"<!--#system_up_time-->",
	"dev_vcc":"<!--#system_vcc-->",
	"iconn_sw":"<!--#system_iconn_sw-->",	
	"power_rs485_1":"<!--#system_power_rs485_1-->",	
	"power_sw1":"<!--#system_power_sw1-->",	
};
var ports = {
	"pstr_":["Eth.1","Eth.2","Eth.3"],
	<!--#system_ports-->
};

var rs_baudrate_sel_val =[
	["1200","1,2 Kbit"],
	["1800","1,8 Kbit"],
	["2400","2,4 Kbit"],
	["4800","4,8 Kbit"],
	["9600","9,6 Kbit"],
	["14400","14,4 Kbit"],
	["19200","19,2 Kbit"],
	["28800","28,8 Kbit"],
	["38400","38,4 Kbit"],
	["56000","56,0 Kbit"],
	["57600","57,6 Kbit"],
	["115200","115,2 Kbit"],
	["230400","230,4 Kbit"],
	["460800","460,8 Kbit"],
	["921600","921,6 Kbit"],
	["1000000","1 Mbit"],
	["2000000","2 Mbit"],
	["3000000","3 Mbit"],
	["4000000","4 Mbit"],
	["5000000","5 Mbit"],
];

var port_src = "<!--#sw_src_port-->";

var DAYS = ["дней","день","дня"];
var HOURS = ["часов","час","часа"];
var SECONDS = ["секунд","секунд","секунды"];
var MINUTES = ["минут","минута","минуты"];

var getWorkTime = function(seconds){
	var dselect = Math.floor(seconds/(3600*24));
	var hselect = Math.floor(seconds/3600)%24;
	var mselect = Math.floor((seconds%3600)/60);
	var sselect = Math.floor(seconds%60);
	
	if( sselect > 5 && sselect < 20 ) {
		sselect = 0;
	} else if(sselect%10 == 0) {
		sselect = 0;
	} else if(sselect%10 == 1) {
		sselect = 1;
	} else if(sselect%10 <= 4) {
		sselect = 2;
	} else {
		sselect = 0;
	}
	
	if( mselect > 5 && mselect < 20 ) {
		mselect = 0;
	} else if(mselect%10 == 0) {
		mselect = 0;
	} else if(mselect%10 == 1) {
		mselect = 1;
	} else if(mselect%10 <= 4) {
		mselect = 2;
	} else {
		mselect = 0;
	}
	
	
	if( hselect > 5 && hselect < 20 ) {
		hselect = 0;
	} if(hselect%10 == 0){
		hselect = 0;
	} else if(hselect%10 == 1) {
		hselect = 1;
	} else if(hselect%10 <= 4) {
		hselect = 2;
	} else {
		hselect = 0;
	}
	
	if( dselect%100 > 5 && dselect%100 < 20) {
		dselect = 0;
	} if(dselect%10 == 0){
		dselect = 0;
	} else if(dselect%10 == 1) {
		dselect = 1;
	} else if(dselect%10 <= 4) {
		dselect = 2;
	} else {
		dselect = 0;
	}
	return Math.floor(seconds/(3600*24)) + ' ' +DAYS[dselect] + ' ' +  Math.floor((seconds%(3600*24))/3600) + ' ' +HOURS[hselect] + ' ' +  Math.floor(seconds%3600/60) + ' ' +MINUTES[mselect];
};

var getIConnState = function(state){
	var sw1 = ((state&1) > 0)?'вкл.':'выкл.';
	var sw2 = ((state&2) > 0)?'вкл.':'выкл.';
	
	return 'Связь 1<->2: ' + sw1 + '<->' + sw2;
};
function set_innerHTML_by_id(id,text){
	if(document.getElementById(id))
	{
		document.getElementById(id).innerHTML=text;
	}
}
function delay_set(id,text,time){
	setTimeout(set_innerHTML_by_id,time,id,text);
}

function GetSpeed(iSpeed){
	var cSpeed = ""; //Строка скорости RS485
	var cIsSpeed = false;
	for (j = 0; j < rs_baudrate_sel_val.length; ++j) {
		if (rs_baudrate_sel_val[j][0] == iSpeed) {
			cSpeed = rs_baudrate_sel_val[j][1];
			cIsSpeed = true;
			break;
		}
	}
	if (cIsSpeed)
		return cSpeed;
	else
		return "";
}

function window_onload(){
	Object.keys(txt).forEach(function(id){
		if("uptime"==id){
			if(document.getElementById(id)) document.getElementById(id).innerHTML=getWorkTime(txt[id]);
			}else if("dev_vcc"==id){
				var cVcc = document.getElementById(id);
				if (cVcc) {
					var cValue = Number(txt[id]);
					if (cValue == 0) {
						cVcc.style.color = 'black';
						cVcc.innerHTML = "[Не измеряется]";
					} else {
						if ((cValue < 8) || (cValue > 16.0)) {
							cVcc.style.color = 'red';
						} else {
							if (cValue < 10.5) {
								cVcc.style.color = 'yellow';
							} else {
								cVcc.style.color = 'black';
							}
						}
						cVcc.innerHTML=txt[id] + " В";
					}
				}
		}else if("iconn_sw"==id){
			if(document.getElementById(id)) document.getElementById(id).innerHTML=getIConnState(txt[id]);
		}else if(("power_modem"==id)){
			if(document.getElementById(id)) document.getElementById(id).innerHTML=((txt[id]=="1")?"Вкл":"Выкл");;
		}else if(("power_sw1"==id)||("power_rs485_1"==id)){
			if(document.getElementById(id)) {
				document.getElementById(id).innerHTML=((txt[id]=="1")?"Есть":"Нет");
				document.getElementById(id).style.color=((txt[id]=="1")?'green':'red')
			}
		}else if(document.getElementById(id)) {
			document.getElementById(id).innerHTML=txt[id];
		}
	});
	Object.keys(ports).forEach(function(id){
		var time=10;
		var i=0;Object.keys(ports[id]).forEach(function(it){
			++i;
			if(document.getElementById(id+i)){
				if(ports[id][it]=="-"){
					document.getElementById(id+i).innerHTML="-";
				}else if(id=="pconn_"){
					//document.getElementById(id+i).innerHTML="<img src=\"img/port_00.gif\"></img>";
					var text="";
					if(i>=4){		// было 3 , у нас 3 порта все они медь
						if(port_src == i){
							text="<img src=\"img/port_12.gif\"></img>";
						} else if(ports["pen_"][it]>0){
							text="<img src=\"img/" + ((ports[id][it]>0)?"port_13":"port_11") + ".gif\"></img>";
							//document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"port_13":"port_11") + ".gif\"></img>";
						} else {
							text="<img src=\"img/port_10.gif\"></img>";
							//document.getElementById(id+i).innerHTML="<img src=\"img/port_10.gif\"></img>";
						}
					} else {
						if(port_src == i){
							text="<img src=\"img/port_02.gif\"></img>";
						} else if(ports["pen_"][it]>0){
							text="<img src=\"img/" + ((ports[id][it]>0)?"port_03":"port_01") + ".gif\"></img>";
							//document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"port_03":"port_01") + ".gif\"></img>";
						} else {
							text="<img src=\"img/port_00.gif\"></img>";
							//document.getElementById(id+i).innerHTML="<img src=\"img/port_00.gif\"></img>";
						}
					}
					delay_set(id+i,"<img src=\"img/port_00.gif\"></img>",time);time+=30;
					delay_set(id+i,text,time);time+=30;
				}else if(id=="pcflw_" || id=="pauto_" || id=="sconn_"){
					document.getElementById(id+i).innerHTML=((ports[id][it]=="1")?"Вкл":"Выкл");
				}else if(id=="pduplex_"){
					document.getElementById(id+i).innerHTML=((ports[id][it]=="1")?"FDX":"HDX");
				}else if(id=="pen_"){
					delay_set(id+i,"<img src=\"img/" + ((ports[id][it]=="1")?"ok":"nok") + ".png\"></img>",time);time+=100;
					//document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"ok":"nok") + ".png\"></img>";
				}else if(id=="pspeed_"){
					document.getElementById(id+i).innerHTML=(ports[id][it]=="1")?"100Мбит":"10Мбит";
				}else if(id=="spbo_"){
					document.getElementById(id+i).innerHTML=((Number(ports[id][it])>30 || Number(ports[id][it]<0))?"Автоматически":ports[id][it]);
					}else if(id=="rs_baudrate_"){
						document.getElementById(id+i).innerHTML = GetSpeed(ports[id][it]);
						//((Number(ports[id][it])>30 || Number(ports[id][it]<0))?"Автоматически":ports[id][it]);
					}else if(id=="ip_port_local_"){
						document.getElementById(id+i).innerHTML = ports[id][it];
					}else if(id=="rs_conn_"){
						document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"portrs_2":"portrs_1") + ".gif\"></img>";
					}else if(id=="rs_tx_"){
						document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"portrs_2":"portrs_1") + ".gif\"></img>";
					}else if(id=="rs_rx_"){
						document.getElementById(id+i).innerHTML="<img src=\"img/" + ((ports[id][it]=="1")?"portrs_2":"portrs_1") + ".gif\"></img>";
				}else{
					document.getElementById(id+i).innerHTML=ports[id][it];
				}
			}
		});
	});
}