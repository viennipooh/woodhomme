var ports = {
	"pstr_":["Оптика","Оптика","Оптика","Оптика","Медь","Медь","Медь",],
	<!--#vl_system_ports-->
};
var vlans = {
	"pstr_":["1","2","3","4","5","6","7",],
	<!--#vl_system_vlans-->
};
var vlvlan_on = "<!--#vl_vlan_on-->";
var vluni_off = "<!--#vl_uni_off-->";
var vlchg_vid_on = "<!--#vl_chgvid_on-->";

function disable_element(element, index, array){
	element.disabled = true;
}
function window_onload(){
	//Общие для VLAN
	var vl_lan;
	vl_lan = document.getElementsByName('vlan_on');
	if (vl_lan.length > 0)
		if (vlvlan_on == '1')
			vl_lan[1].checked = true;
		else
			vl_lan[0].checked = true;
	vl_lan = document.getElementsByName('uni_off');
	if (vl_lan)
		if (vluni_off == '1')
			vl_lan[1].checked = true;
		else
			vl_lan[0].checked = true;
	vl_lan = document.getElementsByName('chg_vid_on');
	if (vl_lan)
		if (vlchg_vid_on == '1')
			vl_lan[1].checked = true;
		else
			vl_lan[0].checked = true;
	//Порты
	Object.keys(ports).forEach(function(id){
		var i = 0;
		var sel_names = document.getElementsByName(id + 'm')[0];
		if(sel_names) {
			var cInp = parseInt(ports[id][0]);
			var cMem;
			for (var k = 0; k < 7; ++k) {
				cMem = document.getElementById(id + (k + 1));
				if (cMem) {
					if ((1 << k) & cInp) cMem.checked = true;
					else cMem.checked = false;
				}
			}
		} else {
			Object.keys(ports[id]).forEach(function(it){
				++i;
				sel_names = document.getElementsByName(id + i);
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
								// Здесь - распаковка в 7 checkbox
								for (var k = 0; k < 7; ++k) {
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
		}
	});
	//VLANы
	Object.keys(vlans).forEach(function(id){
		var i = 0;
		Object.keys(vlans[id]).forEach(function(it){
			++i;
			var sel_names = document.getElementsByName(id + i);
			if(sel_names.length > 0){
				if(vlans[id][it]=="-"){
					for(j=0;j<sel_names.length;j++){
						sel_names[j].disabled = true;
					}
				}else{
					for(j=0;j<sel_names.length;j++){
						sel_names[j].disabled = false;
						if(sel_names[j].type == "radio"){
							if(sel_names[j].value == vlans[id][it]) sel_names[j].checked = true;
							else sel_names[j].checked = false;
						}else if(sel_names[j].type == "checkbox"){
							if("0" == vlans[id][it]) sel_names[j].checked = false;
							else sel_names[j].checked = true;
						}else if(sel_names[j].type == "hidden"){
							// Здесь - распаковка в 7 checkbox
							for (var k = 0; k < 7; ++k) {
								var cMem = document.getElementById(id + i + '_' + (k + 1));
								if (cMem) {
									if (!cMem.disabled) {
										var cInp = parseInt(vlans[id][it]);
										var cMask = (1 << k);
										if (cMask & cInp) cMem.checked = true;
										else cMem.checked = false;
									}
								}
							}
							if("0" == vlans[id][it]) sel_names[j].checked = false;
							else sel_names[j].checked = true;
						}else if(sel_names[j].type == "text"){
							sel_names[j].value = vlans[id][it];
						}
					}
				}
			}
		});
	});
	
	var inp = document.getElementsByTagName('input');
	for(var i in inp){
		// if(inp[i].name == "pen_"+port_src) {
			// inp[i].disabled = true;
		// }
		if(inp[i].type == "submit" || inp[i].type == "button"){
			inp[i].disabled = false;
		}
	}
}

var member = [
	"pv_member_",
	"vlan_member_"
];
var mem_num = [
	7,
	3
];
var log_var = [
	"otag_ins_",
	"otag_del_",
	"no_v_pack_del_",
	"no_p_vid_pack_del_"
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
	var cItem;
	for (var i = 0; i < log_var.length; ++i) {
		var cVar = 0;
		for (var j = 0; j < 7; ++j) {
			cItem = document.getElementById(log_var[i] + (j + 1));
			if (cItem) {
				if (cItem.checked)
					cVar |= (1 << j);
			}
		}
		cItem = document.getElementsByName(log_var[i] + 'm')[0];
		if (cItem) {
			cItem.value = cVar;
		}
	}
	
	var subm = document.getElementsByName("applying")[0];
	if (subm) {
		subm.value = "1";
		this.submit;
	}
}