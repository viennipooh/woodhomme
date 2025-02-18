var ports = {
	"pstr_":["Медь","Медь","Оптика","Оптика"],
	<!--#system_ports-->
};
var ip_mode_sel_val =[
	["modbusgw","ModBus TCP-RTU Шлюз"],
	["modbus_rtu_over_tcp","RAW over TCP Шлюз"],		
	["modbus_gw_udp","ModBus UDP-RTU Шлюз"],
	["modbus_rtu_over_udp","RAW over UDP Шлюз"],		
	["tcpclient","RAW over TCP Клиент"],		
];
function disable_element(element, index, array){
	element.disabled = true;
}
function window_onload(){
	for(i=0;i<1;i++)
	{
		var list=document.getElementsByName("ip_mode_"+(i+1));
		if(list){
			var j=0;ip_mode_sel_val.forEach(function(item,i,arr){
				var opt = document.createElement("option");
				opt.text = ""+item[1];
				opt.value = item[0];
				if(i==0)
				  opt.selected = 1;
				list[0].add(opt);
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
function sm1(){
	var subm = document.getElementsByName("applying")[0];
	if (subm) {
		subm.value = "1";
		this.submit;
	}
}
