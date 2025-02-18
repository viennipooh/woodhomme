var regs = {
	"pstr_":["0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39"],
	<!--#usart_regs-->
};

//Код ошибки
var rs_rx_error = "<!--#rs_rxerror-->";
//Начальный регистр
var rs_start_reg_1 = "<!--#rs_startreg-->";

//Сигнал пуска поллинга
// var rs_start_poll = "<!--#rs_startpoll-->";

//Сообщения об ошибках. Массив редактируется совместно с enum eMBResult
var rs_error_msg = [
  ["∙ ∙ ∙"],
  ["Кадр принят"],
  ["Таймаут (нет кадра)"],
  ["Ошибка CRC"],
  ["Ошибка MODBUS"],
];

function window_onload(){
	//Загрузка считываемых регистров usart
	Object.keys(regs).forEach(function(id){
		var i = 0;
		Object.keys(regs[id]).forEach(function(it) {
			if(document.getElementById(id+i)) {
				document.getElementById(id+i).innerHTML = " " + (regs[id][it]);
			} else {
				document.getElementById("reg_" + i).innerHTML = " " + (regs[id][it]);
			}
			++i;
		});
	});
	var cErr = document.getElementById("err_0");
	if (cErr)
		cErr.innerHTML = rs_error_msg[rs_rx_error];
	var i;
	var cAddr = parseInt(rs_start_reg_1, 10);
	var cCurrAddr;
	for (i = 0; i < 4; ++i) {
		var cAddrItem = document.getElementById("addr_" + i);
		if (cAddrItem) {
			cCurrAddr = cAddr + 10 * i;
			cAddrItem.innerHTML = " " + cCurrAddr;
		}
	}
	window.parent.postMessage("" + rs_rx_error, '*');
}
