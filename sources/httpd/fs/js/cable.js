var txt = {
	"dev_name":"<!--#system_device_name-->",
	<!--#system_network-->,
};
var ports = {
	"pstr_":["Eth.5","Eth.6","Eth.7"],
	<!--#cable_link_md-->
};

var CableState = [
  "Норма",
  "Обрыв",
  "КЗ",
  "Ошибка диагностики",
  "Не проверен",
];

var port_src = "<!--#sw_src_port-->";

function set_innerHTML_by_id(id,text){
	if(document.getElementById(id))
	{
		document.getElementById(id).innerHTML=text;
	}
}
function delay_set(id,text,time){
	setTimeout(set_innerHTML_by_id,time,id,text);
}


function window_onload(){
	Object.keys(ports).forEach(function(id){
		var time=10;
		var i=0;
		Object.keys(ports[id]).forEach(function(it){
			++i;
			if(document.getElementById(id+i)){
				if(ports[id][it]=="-"){
					document.getElementById(id+i).innerHTML="-";
				}else if(id=="cable_state"){
					document.getElementById(id+i).innerHTML=CableState[ports[id][it]];
				}else if(id=="cable_length"){
					if ((ports["cable_state"][it] == 0) || (ports["cable_state"][it] == 3) || (ports["cable_state"][it] == 4)) {
						document.getElementById(id+i).innerHTML="---";
					} else {
						document.getElementById(id+i).innerHTML=ports[id][it];
					}
				}else{
					document.getElementById(id+i).innerHTML=ports[id][it];
				}
			}
		});
	});
}