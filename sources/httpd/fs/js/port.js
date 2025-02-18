var ports = {
	"pstr_":["Медь","Медь","Оптика","Оптика"],
	<!--#system_ports-->
};
var port_src = "<!--#sw_src_port-->";
function disable_element(element, index, array){
	element.disabled = true;
}
function window_onload(){
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
						}else if(sel_names[j].type == "hidden"){
							// Здесь - распаковка в 4 checkbox
							for (var k = 0; k < 4; ++k) {
								var cMem = document.getElementById(id + i + '_' + (k + 1));
								if (cMem) {
									if (!cMem.disabled) {
										var cInp = parseInt(ports[id][it]);
										var cMask = (1 << k);
										if (cMask & cInp) cMem.checked = true;
										else cMem.checked = false;
									}
								}
							}
							if("0" == ports[id][it]) sel_names[j].checked = false;
							else sel_names[j].checked = true;
						}else if(sel_names[j].type == "text"){
							sel_names[j].value = ports[id][it];
						}
					}
				}
			}
		});
	});
	var inp = document.getElementsByTagName('input');
	for(var i in inp){
		if(inp[i].name == "pen_"+port_src) {
			inp[i].disabled = true;
		}
		if(inp[i].type == "submit" || inp[i].type == "button"){
			inp[i].disabled = false;
		}
	}
}

var member = [
	"subnet_member_"
];
var mem_num = [
	3
];

function packMembers(iMember, iMembers) {
	var cMem = document.getElementsByName(iMembers)[0];
	if (cMem) {
		cMem.value = parseInt('0', 10);
		for (var i = 0; i < 7; ++i) {
			var cCheck = document.getElementById(iMember + (i + 1));
			if (cCheck) {
				if (cCheck.checked) {
					cMem.value |= (1 << i);
				}
			}
		}
	}
}

function sm1(){
	//Упаковать Members для портов и для VLAN
	for (var i = 0; i < member.length; ++i) {
		for (var j = 0; j < mem_num[i]; ++j) {
			packMembers(member[i] + (j + 1) + '_', member[i] + (j + 1));
		}
	}
}