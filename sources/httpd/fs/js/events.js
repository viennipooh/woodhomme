var txt = {
	"rs_dev_addr_1":"<!--#rs_devaddr-->",
};

var rs_modbus_func =[
	["3","[03] Read Holding Registers"],
	["4","[04] Read Input Registers"],
];

var regs = {
	"pstr_":["0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19"],
	<!--#usart_regs-->
};

//Код ошибки
var rs_rx_error = "<!--#rs_rxerror-->";
//Сигнал пуска поллинга
// var rs_start_poll = "<!--#rs_startpoll-->";
// var rs_stop_poll = "<!--#rs_stoppoll-->";
var v_rs_tx_count = "<!--#rs_txcount-->";
var v_rs_err_count = "<!--#rs_errcount-->";

//Сообщения об ошибках. Массив редактируется совместно с enum eMBResult
// var rs_error_msg = [
  // ["∙ ∙ ∙"],
  // ["Кадр принят"],
  // ["Таймаут (нет кадра)"],
  // ["Ошибка CRC"],
  // ["Ошибка MODBUS"],
// ];

// var getIConnState = function(state){
	// var sw1 = ((state&1) > 0)?'вкл.':'выкл.';
	// var sw2 = ((state&2) > 0)?'вкл.':'выкл.';
	
	// return 'Связь 1<->2: ' + sw1 + '<->' + sw2;
// };
function set_innerHTML_by_id(id,text){
	if(document.getElementById(id))
	{
		document.getElementById(id).innerHTML=text;
	}
}
function delay_set(id,text,time){
	setTimeout(set_innerHTML_by_id,time,id,text);
}

var cTimerId;
var ciErrCount;
var ciRxCount;
var cVErrCount;
var cRxCount;
var cStop = 0;

function StartInterval(){
	var cPeriod = "<!--#rs_pollperiod-->";
	cTimerId = setInterval(function() {
		document.getElementById('devframe').src = document.getElementById('devframe').src;
	}, cPeriod);
}

// function listener(e) {
	// if (rs_start_poll == '0')
		// return;
	// if (e.data > '1') {
		// ciErrCount++;
		// if (cVErrCount) {
			// cVErrCount.innerHTML = "" + ciErrCount;
		// }
	// }
	// if (cRxCount) {
		// ciRxCount++;
		// cRxCount.innerHTML = "" + ciRxCount;
	// }
// }

function window_onload(){
	var i = 0;
	// var cErr = document.getElementById("err_0");
	// if (cErr)
		// cErr.innerHTML = rs_error_msg[rs_rx_error];
	Object.keys(txt).forEach(function(id){
		var cTag;
		if("rs_dev_addr_1"==id){
			cTag = document.getElementsByName(id)[0];
			if (cTag)
				cTag.value = txt[id];
		}else if("rs_modbus_func_1"==id){
			cTag = document.getElementsByName(id)[0];
			if (cTag) {
				cTag.value = txt[id];
			}
		}else if(("rs_start_reg_1"==id)){
			if(document.getElementsByName(id)) document.getElementsByName(id)[0].value=txt[id];
		}else {
			if(document.getElementsByName(id)) document.getElementsByName(id)[0].value=txt[id];
		}
	});
	//Заполнить mbpollack из тега
	// var cSPoll = document.getElementsByName("startpoll");
	// var cStart;
	// if (cSPoll) {
		// cSPoll[0].value = rs_start_poll;
		// cStart = document.getElementsByName("mbpollack");
		// if (cStart.length >= 1) {
			// cStart[0].value = cSPoll[0].value;
			// if (cSPoll[0].value != "0") {
				// StartInterval();
			// }
		// }
		// if (cSPoll[0].value == 1) { //Включен поллинг
			// setInpActive(false);
		// } else {
			// setInpActive(true);
		// }
	// }
	// if (rs_stop_poll == 0) {
		// ciRxCount = 0;
		// ciErrCount = 0;
	// } else {
		// ciRxCount = v_rs_tx_count;
		// ciErrCount = v_rs_err_count;
	// }
	// cRxCount = document.getElementById("tx_count");
	// if (cRxCount) {
		// cRxCount.innerHTML = "" + ciRxCount;
	// }
	// cVErrCount = document.getElementById("err_count");
	// if (cVErrCount) {
		// cVErrCount.innerHTML = "" + ciErrCount;
	// }
    // window.addEventListener("message", listener, false);
	// rs_stop_poll = 0;
}
