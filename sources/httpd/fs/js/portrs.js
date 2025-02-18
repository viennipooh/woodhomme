var ports = {
	"pstr_":["Медь","Медь","Оптика","Оптика"],
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
];

	// ["1000000","1 Mbit"],
	// ["2000000","2 Mbit"],
	// ["3000000","3 Mbit"],
	// ["4000000","4 Mbit"],
	// ["5000000","5 Mbit"],

function disable_element(element, index, array){
	element.disabled = true;
}
function window_onload(){
	for(i=0;i<1;i++)
	{
		var list=document.getElementsByName("rs_baudrate_"+(i+1));
		if(list){
			var j=0;rs_baudrate_sel_val.forEach(function(item,i,arr){
				var option = document.createElement("option");
				option.text = ""+item[1];
				option.value = item[0];
				list[0].add(option);
			});
		}
	}
	Object.keys(ports).forEach(function(id){
		var i=0;Object.keys(ports[id]).forEach(function(it){
			++i;
			var sel_names = document.getElementsByName(id+i);
			if(sel_names.length > 0){
				if(ports[id][it]=="-"){
					for(j=0;j<sel_names.length;j++){
						sel_names[j].disabled = true;
					}
				}else{
					for(j=0;j<sel_names.length;j++){
						sel_names[j].disabled = false;
						if(sel_names[j].type == "radio"){
							if(sel_names[j].value == ports[id][it]) sel_names[j].checked = true;
							else sel_names[j].checked = false;
						}else if(sel_names[j].type == "checkbox"){
							if("0" == ports[id][it]) sel_names[j].checked = false;
							else sel_names[j].checked = true;
						}else if(sel_names[j].type == "text"){
							sel_names[j].value = ports[id][it];
						}else if(sel_names[j].type == "select-one"){
							sel_names[j].value = ports[id][it];
						}
					}
				}
			}
		});
	});
}

function sm0(){
	var subm = document.getElementsByName("applying")[0];
	if (subm) {
		subm.value = "0";
		this.submit;
	}
}

function sm1(){
	var subm = document.getElementsByName("applying")[0];
	if (subm) {
		subm.value = "1";
		this.submit;
	}
}
