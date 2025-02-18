var ping = {
	<!--#system_ping-->
};

var is_assigned = "<!--#tm_is_assigned-->";
var corr_value = "<!--#ping_corr_value-->";
var corr_int = "<!--#ping_corr_int-->";

function set(sf,sv){
	var cItem = document.getElementsByName(sf)[0];
	if (cItem) {
		cItem.value=sv;
	}
}
function choose(sf,sv){document.ping[sf].selectedIndex=sv;}
function window_onload(){
	Object.keys(ping).forEach(function(id){
		var cValue = Number(ping[id]);
		if((id=="start_hour") || (id=="start_min") || (id=="int_min") || (id=="int_sec")) {
			if (cValue < 10) {
				if (cValue >= 0)
					cValue = "0" + cValue;
				else
					cValue = "00";
			} else {
				if (cValue >= 100)
					cValue = "99";
			}
			set(id, cValue);
		}
		
		if(id=="ping_ip") {
			var cItem = document.getElementsByName(id)[0];
			if (cItem) {
				cItem.value=ping[id];
			}
		}
		if((id=="ping_num") || (id=="ping_err_num")) {
			set(id, cValue);
		}
		if((id=="ping_mode") /*|| (id=="hd_wdog")*/) {
			set(id, cValue);
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

function OnTimeChg(){
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
