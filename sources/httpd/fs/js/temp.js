var txt = {
	//<!--#system_network-->,
	"dev_temp":"<!--#system_temp-->",
	"dev_warn_temp":"<!--#system_warn_temp-->",
	"dev_fault_temp":"<!--#system_fault_temp-->",
	"dev_state_temp":"<!--#system_state_temp-->",
};

function set(sf, sv) { document.network[sf].value = sv; }
function window_onload() {
	Object.keys(txt).forEach(function(id){
		if (id != "dev_temp") {
			set(id, txt[id]);
		} else {
			var cTemp = document.getElementById(id);
			if (cTemp) {
				if (txt["dev_state_temp"] <= 10) {
					cTemp.style.color = 'green';
				} else if (txt["dev_state_temp"] == 11) {
					cTemp.style.color = 'yellow';
				} else if (txt["dev_state_temp"] == 12) {
					cTemp.style.color = 'red';
				}
				cTemp.innerHTML = txt[id];
			}
		}
	});
}
function check_value(obj){
	return true;
}
