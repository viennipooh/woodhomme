var time = {
	<!--#system_time-->
};

var is_assigned = "<!--#tm_is_assigned-->";
var corr_value = "<!--#time_corr_value-->";
var corr_int = "<!--#time_corr_int-->";

function set(sf,sv){
	var cItem = document.getElementsByName(sf)[0];
	if (cItem) {
		cItem.value=sv;
	}
}
function choose(sf,sv){document.time[sf].selectedIndex=sv;}
function window_onload(){
	var subm = document.getElementById("id_corr_value");
	if (subm) {
		var cInt = "";
		if ((corr_int) && (corr_int >= 60)) {
			var cMin = parseInt(corr_int / 60);
			cMin = parseInt(cMin % 60);
			var cHour = parseInt(corr_int / 3600);
			if (cHour > 0)
				cInt += cHour + " ч. ";
			cInt += cMin + " м. ";
		}
		subm.innerHTML = corr_value + " ( " + cInt + " )";
	}
	var cAssigned = document.getElementById("assigned");
	if (is_assigned == 0) {
		cAssigned.style.color = 'red';
		cAssigned.innerHTML = "Время не установлено!";
	} else {
		cAssigned.style.color = 'black';
		cAssigned.innerHTML = "Время установлено";
	}
	Object.keys(time).forEach(function(id){
		var cValue = Number(time[id]);
		if((id=="day") || (id=="hour") || (id=="minute")) {
			if (cValue < 10)
				cValue = "0" + cValue;
			set(id,cValue);
		}
		if(id=="month") {
			cValue += 1;
			if (cValue < 10)
				cValue = "0" + cValue;
			set(id,cValue);
		}
		if(id=="year") {
			cValue += 1900;
			set(id,cValue);
		}
		
		if(id=="ntp_ip") {
			var cItem = document.getElementsByName(id)[0];
			if (cItem) {
				cItem.value=time[id];
			}
		}
		if((id=="ntp_mode") || (id=="period_num") || (id=="period_kind") || (id=="time_zone")) {
			set(id,cValue);
		}
		
	});
}
function validate_value2(val, start, end) {
	var cVal = parseInt(val);
	if (cVal >= 0) {
		if ((val.length > 2) || (val.length < 1))
			return(false);
		if ((cVal < start) || (cVal > end))
			return(false);
		return(true);
	}
	return (false);
}
function validate_year(val, start) {
	var cVal = parseInt(val);
	if (cVal >= 0) {
		if (val.length != 4)
			return(false);
		var cVal = parseInt(val);
		if (cVal < start)
			return(false);
		return(true);
	}
	return (false);
}

var max_value = [ //Максимальные значения, соответствующие 49 суткам
  49 * 24 * 60,
  49 * 24,
  49
];

function check_value(obj){
	if(validate_value2(obj["day"].value, 1, 31)==false){
		alert("День должен быть в диапазоне от 1 до 31. !\r\n");
		return false;
	}
	if(validate_value2(obj["month"].value, 1, 12)==false){
		alert("Месяц должен быть в диапазоне от 1 до 12. !\r\n");
		return false;
	}
	if(validate_year(obj["year"].value, 2018)==false){
		alert("Год должен быть не менее 2018. !\r\n");
		return false;
	}
	if(validate_value2(obj["hour"].value, 0, 23)==false){
		alert("Час должен быть в диапазоне от 0 до 23. !\r\n");
		return false;
	}
	if(validate_value2(obj["minute"].value, 0, 59)==false){
		alert("Минута должна быть в диапазоне от 0 до 59. !\r\n");
		return false;
	}
	if (obj["period_num"].value > max_value[obj["period_kind"].value]) {
		switch (obj["period_kind"].value) {
		case 0:
			alert("Период не должен быть больше " + max_value[obj["period_kind"].value] + " минут. !\r\n");
			return false;
		case 1:
			alert("Период не должен быть больше " + max_value[obj["period_kind"].value] + " часов. !\r\n");
			return false;
		case 2:
			alert("Период не должен быть больше " + max_value[obj["period_kind"].value] + " дней. !\r\n");
			return false;
		}
	}
	return true;
}

function SetCurrentTime(){
	var Data = new Date();
	var cDate = Data.toLocaleDateString();
	var cTime = Data.toLocaleTimeString();
	var Year = Data.getFullYear();
	var Month = Data.getMonth();
	var Day = Data.getDate();
	var Hour = Data.getHours();
	var Minutes = Data.getMinutes();
	var Seconds = Data.getSeconds();
	if (Minutes < 59) {
		if (Seconds > 30)
			Minutes++;
	}
	var cItem;
	cItem = document.getElementsByName("minute")[0];
	if (cItem)
		cItem.value = Minutes;
	cItem = document.getElementsByName("hour")[0];
	if (cItem)
		cItem.value = Hour;
	cItem = document.getElementsByName("day")[0];
	if (cItem)
		cItem.value = Day;
	cItem = document.getElementsByName("month")[0];
	if (cItem)
		cItem.value = Month + 1;
	cItem = document.getElementsByName("year")[0];
	if (cItem)
		cItem.value = Year;
}

function OnTimeChg(){
	var subm = document.getElementsByName("setparam")[0];
	if (subm) {
		subm.value = "0";
	}
	this.submit;
}
function OnCurrTimeChg(){ //Установить текущее время компьютера
	SetCurrentTime();
	
	var subm = document.getElementsByName("setparam")[0];
	if (subm) {
		subm.value = "0";
	}
	this.submit;
}
function OnParamChg(){
	var subm = document.getElementsByName("setparam")[0];
	if (subm) {
		subm.value = "1";
	}
	this.submit;
}
