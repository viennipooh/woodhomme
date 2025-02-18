var events = {
	<!--#events_tab-->
};

var event_count = "<!--#time_ev_count-->";
var event_pos = "<!--#time_ev_pos-->";

//События, роли (массивы)
var log_event_type = [
  ["Общее"],
  ["Системное"],
  ["Устройство"],
  ["Питание"],
  ["Уставка"],
];

var log_event_name = [
  ["Пуск устройства "],
  ["Останов устройства"],
  ["Напряжение питания меньше "],
  ["Напряжение питания больше "],
  ["Установка системного времени "],
  ["Установка системного времени (NTP) "],
  ["Изменение значения уставки "],
  ["Изменение значения уставки порта "],
  ["Контроль системного времени (NTP) "],
  ["Загружено обновление ПО: "],
  ["Температура меньше "],
  ["Температура больше "],
  ["Пересброс порта "],
  ["Пересброс свитча "],
  ["Перезагрузка по потере связи "],
  ["Порт "],
  ["Порт "],
];

var log_event_name_end = [
  [""],
  [""],
  [" В"],
  [" В"],
  [""],
  [""],
  [""],
  [""],
  [""],
  [""],
  ["°"],
  ["°"],
  [" по потере связи"],
  [" по потере связи"],
  [""],
  [" есть связь"],
  [" нет связи"],
];

//Имена уставок и значений портов Ethernet
var port_name = [
  ["F.Eth.1"],
  ["F.Eth.2"],
  ["F.Eth.3"],
  ["F.Eth.4"],
  ["Eth.5"],
  ["Eth.6"],
  ["Eth.7"],
  [" "],
  [" "],
  [" "],
  ["RS485 1"],
  ["RS485 2"],
];

var port_param_name = [
  ["Подключен"],
  ["Авто"],
  ["Скорость"],
  ["Дуплекс"],
  ["Упр. потоком"],
  ["MDI/MDIX Авто"],
  ["MDI/MDIX"],
  [""],
  [""],
  [""],
  ["Чётность"],
  ["Стоп биты"]
];

var port_param_value_name = [
  ["Выкл", "Вкл"],
  ["Выкл", "Вкл"],
  ["10Мбит", "100Мбит"],
  ["HalfDX", "FullDX"],
  ["Выкл", "Вкл"],
  ["MDI ", "MDI Авто"],
  ["MDI", "MDIX"],
  ["", ""],
  ["", ""],
  ["", ""],
  ["none", "even", "odd"],
  ["0", "1", "2"]
];


//Имена уставок и значений портов RS485
var rs_port_name = [
  ["1 RS485"],
  ["2 RS485"]
];

var rs_param_name = [
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

var cPage = 30;
function next_row() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		var cVal = parseInt(subm.value) + 1;
		if (subm.value < (event_count - cPage)) {
			subm.value = cVal;
		}
		this.submit;
	}
}

function prev_row() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		var cVal = parseInt(subm.value) - 1;
		if (subm.value > 0) {
			subm.value = cVal;
		}
		this.submit;
	}
}

function next_page() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		var cVal = parseInt(subm.value) + cPage;
		if (subm.value < (event_count - (2 * cPage))) {
			subm.value = cVal;
		} else {
			subm.value = (event_count - cPage);
		}
		this.submit;
	}
}

function prev_page() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		var cVal = parseInt(subm.value) - cPage;
		if (subm.value >= cPage) {
			subm.value = cVal;
		} else {
			subm.value = 0;
		}
		this.submit;
	}
}

function to_end() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		subm.value = (event_count - cPage);
		this.submit;
	}
}

function to_begin() {
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		subm.value = 0;
		this.submit;
	}
}

function window_onload(){
	var subm = document.getElementsByName("tab_pos")[0];
	if (subm) {
		subm.value = event_pos;
	}
	//Загрузка считываемых регистров usart
	var cE_Tab = document.getElementById("e_tab");
	if (cE_Tab) {
		Object.keys(events).forEach(function(id){
			var row = cE_Tab.tBodies[0].insertRow(-1);
			var i = 0;
			Object.keys(events[id]).forEach(function(it) {
				if (i < 5) {
					var cell = row.insertCell(-1);
					if (i == 2) {
						var cType;
						if (events[id][it] === "255")
							cType = "< . . . >";
						else
							cType = log_event_type[events[id][it]];
						cell.innerHTML = cType;
					} else if (i == 3) {
						cell.style.textAlign = "left";
						if (events[id][it] === "65535") {
							cell.innerHTML = "< . . . >";
						} else {
							if ((events[id][3] == 0) || (events[id][3] == 14) || //Пуск и Перезагрузка по связи
								 (events[id][3] == 15) || (events[id][3] == 16)) { //Порт х: Есть связь / Нет связи 
								cell.innerHTML = log_event_name[events[id][3]] + events[id][4] + log_event_name_end[events[id][3]];
							} else if ((events[id][3] == 12) || (events[id][3] == 13)) { //Пуск и Перезагрузка по связи
								cell.innerHTML = log_event_name[events[id][3]] + events[id][5] + log_event_name_end[events[id][3]];
							} else if ((events[id][3] == 2) || (events[id][3] == 3)) {
								cell.innerHTML = log_event_name[events[id][3]] + events[id][4] + log_event_name_end[events[id][3]];
							} else if ((events[id][3] == 4) || (events[id][3] == 5) || (events[id][3] == 8)) {
								var cDiff = events[id][5];
								var cSign = "";
								var cMax = 6000000; //= 100 000 минут
								var cNoSec = false;
								if (cDiff < 0) {
									if (cDiff == -cMax) {
										cSign = "<= ";
										cNoSec = true;
									}
									cSign += "-";
									cDiff = -cDiff;
								} else if (cDiff > 0) {
									if (cDiff == cMax) {
										cSign = ">= ";
										cNoSec = true;
									}
									cSign += "+";
								}
								if ((cDiff < 60) && (cDiff > -60)) {
									cStr = cSign + cDiff + " сек";
								} else {
									if (cNoSec) {
										cStr = cSign + parseInt(cDiff / 60) + " мин";
									} else {
										cStr = cSign + parseInt(cDiff / 60) + " мин " + parseInt(cDiff % 60) + " сек";
									}
								}
								cell.innerHTML = log_event_name[events[id][3]] + cStr + log_event_name_end[events[id][3]];
							} else if (events[id][3] == 7) { //Параметры порта
								cell.innerHTML = log_event_name[events[id][3]] + port_name[events[id][5]] + " : " + port_param_name[events[id][6]] + " " + log_event_name_end[events[id][3]];
							} else if ((events[id][3] == 9)) { //Загружено обновление ПО
								cell.innerHTML = log_event_name[events[id][3]] + "Версия " + events[id][4] + " от " + events[id][5];
							} else if ((events[id][3] == 10) || (events[id][3] == 11)) {
								cell.innerHTML = log_event_name[events[id][3]] + events[id][5] + log_event_name_end[events[id][3]];
							} else {
								cell.innerHTML = log_event_name[events[id][3]] + log_event_name_end[events[id][3]];
							}
						}
					} else if (i == 4) {
						if (events[id][it] === "-nan") {
							cell.innerHTML = "< . . . >";
						} else {
							switch (events[id][3]) {
							case "7":
								if ((events[id][5] >= 10) && (events[id][5] <= 11)) {
									if (events[id][6] == 2) {
										var cSpeed = " ";
										var cIsSpeed = false;
										for (j = 0; j < rs_param_name.length; ++j) {
											if (rs_param_name[j][0] == events[id][4]) {
												cSpeed = rs_param_name[j][1];
												cIsSpeed = true;
												break;
											}
										}
										if (cIsSpeed)
											cell.innerHTML = cSpeed;
									} else {
										cell.innerHTML = port_param_value_name[events[id][6]][events[id][4]];
									}
								} else {
									cell.innerHTML = port_param_value_name[events[id][6]][events[id][4]];
								}
								break;
							case "12":
							case "13":
							case "15":
							case "16":
								cell.innerHTML = events[id][4];
								break;
							default:
								cell.innerHTML = events[id][4] + log_event_name_end[events[id][3]];
								break;
							}
						}
					} else {
						cell.innerHTML = events[id][it];
					}
				}
				i += 1;
			});
		});
	}
}
