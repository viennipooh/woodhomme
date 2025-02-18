var sw_item = {
	"pstr_":["Оптика","Оптика","Оптика","Оптика","Медь","Медь","Медь",],
	<!--#stp_system_rstp-->
};
var ports = {
	"pstr_":["1","2","3","4",],
	<!--#stp_ports-->
};

var stp_sw_role_arr =[
	["Корневой"],
	["Не корневой"],
];

var stp_port_role_arr =[
	["---"],			//NONE,
	["Инициализация"],	//INIT_PORT,
	["Отключение"],		//DISABLE_PORT,
	["Отключен"],		//DISABLED_PORT,
  
	["Корневой"],		//ROOT_PORT,
	["Предложен"],		//ROOT_PROPOSED,
	["Согласован"],		//ROOT_AGREED,
	["Корневой"],		//REROOT,
	["Корневой"],		//ROOT_FORWARD,
	["Корневой"],		//ROOT_LEARN,
	["Корневой"],		//REROOTED,
  
	["Назначен"],		//DESIGNATED_PORT,
	[". . ."],			//DESIGNATED_PROPOSE,
	["Назначен"],		//DESIGNATED_SYNCED,
	["Назначен"],		//DESIGNATED_RETIRED,
	["Назначен"],		//DESIGNATED_FORWARD,
	["Назначен"],		//DESIGNATED_LEARN,
	["Назначен"],		//DESIGNATED_DISCARD,

	["Альтернативный"],	//ALTERNATE_PORT,
	["Альтернативный"],	//ALTERNATE_PROPOSED,
	["Альтернативный"],	//ALTERNATE_AGREED,
	["Блокирован"],		//BLOCK_PORT,
	["Резервный"],		//BACKUP_PORT,
];


var stp_port_state_arr =[
	["---"],		//NONE
	["Отключен"],	//DISCARDING
	["Обучение"],	//LEARNING
	["Включен"],	//FORWARDING
];

var stp_port_rx_arr =[
	["---"],			//NONE,
	["Нет кадров"],		//DISCARD,
	["Прием"],			//RECEIVE,
	["Пауза приема"],	//NO_RECEIVE,
];

var port_name = [
	[". . ."],
	["Eth1"],
	["Eth2"],
	["F.Eth3"],
	["F.Eth4"],
];

function disable_element(element, index, array){
	element.disabled = true;
}

var cRstpOn;
function window_onload(){
	Object.keys(sw_item).forEach(function(id){
		//Параметры
		if (id=="rstp_on") {
			document.getElementById(id).value = sw_item[id];
			cRstpOn = sw_item[id];
		}
		if (id=="hw_version2") {
			var Version = sw_item[id];
			if (Version != "0") {
				document.getElementById("rstp_on").disabled = true;
				document.getElementById("sw_priority").disabled = true;
				document.getElementById("setup_recover").disabled = true;
				document.getElementById("setup_set").disabled = true;
				
			}
		}
		if ((id=="mac_addr") || (id=="sw_priority") || (id=="owner_id") || (id=="max_age") || (id=="hello_time") || (id=="tx_hold_count")) {
			document.getElementById(id).value = sw_item[id];
		}
		//Состояния
		if (id=="stp_sw_role") {
			document.getElementById("sw_role").innerHTML = stp_sw_role_arr[sw_item[id]];
		}
		if (id=="stp_root_port_num") {
			document.getElementById("root_port_num").innerHTML = port_name[sw_item[id]];
		}
		if (id=="stp_root_bridge") {
			document.getElementById("root_bridge").innerHTML = sw_item[id];
		}
	});
	Object.keys(ports).forEach(function(id){
		var i = 0;
		Object.keys(ports[id]).forEach(function(it){
			++i;
			var sel_names = document.getElementById(id + "_" + i);
			if(sel_names){
				//Параметры
				if (sel_names.type == "checkbox")
					sel_names.checked = (ports[id][it] != 0);
				if (sel_names.type == "text")
					sel_names.value = ports[id][it];
				//Состояния
				if (id == "port_role")
					sel_names.innerHTML = stp_port_role_arr[ports[id][it]];
				if (id == "port_state")
					sel_names.innerHTML = stp_port_state_arr[ports[id][it]];
				if (id == "port_rx")
					sel_names.innerHTML = stp_port_rx_arr[ports[id][it]];
			}
		});
	});
	
}

var member = [
	"enabled_",
	"admin_edge_",
	"auto_edge_"
];
var mem_num = [
	4,
	4,
	4
];

function sm0(){
	var subm = document.getElementById("id_applying");
	if (subm) {
		subm.value = "0";
		this.submit;
	}
}

var sec_max = 25;
var timer_val = 500;
var seconds;
var timerId;

function updateProgress() {
		--seconds;
		var cPers = document.getElementById("percentUploaded");
		if (seconds > 0) {
			if (cPers) {
				var cSec = parseInt(seconds * timer_val / 1000);
				cPers.innerHTML = "!!! Перезагрузка устройства (" + cSec.toString() + " сек)";
				cPers.style.color = 'red';
			}
		} else {
			clearInterval(timerId);
			if (cPers) {
				cPers.innerHTML = "";
				cPers.style.color = 'black';
			}
		}
}

function formSubmit() {
	var cIsRstpOn = document.getElementById("rstp_on").value;
	if (cIsRstpOn != cRstpOn) {
		if (cIsRstpOn)
			sec_max = 25;
		else
			sec_max = 10;
		seconds = sec_max * 1000 / timer_val;
		percent = 0;
		timerId = setInterval('updateProgress()', timer_val);
		return true;
	}
}

function sm1(){
	//Упаковать Members для портов и для VLAN
	for (var i = 0; i < member.length; ++i) {
		var cMem = document.getElementById(member[i] + 'm');
		var cVar = 0;
		for (var j = 0; j < mem_num[i]; ++j) {
			var cCheck = document.getElementById(member[i] + (j + 1));
			if (cCheck) {
				if (cCheck.checked) {
					cVar |= (1 << j);
				}
			}
		}
		if (cMem)
			cMem.value = cVar;
	}
	var subm = document.getElementById("id_applying");
	if (subm) {
		formSubmit();
		subm.value = "1";
		this.submit;
	}
}

