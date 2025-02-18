var network={
	"ipconfig":"static",
	<!--#system_network-->
	/*"ip":"192.168.1.200",
	"netmask":"255.255.255.0",
	"gateway":"192.168.1.1",
	"mac":"00:01:02:03:04:05",*/
};
function set(sf,sv){document.network[sf].value=sv;}
function choose(sf,sv){document.network[sf].selectedIndex=sv;}
function window_onload(){
	Object.keys(network).forEach(function(id){
		if(id=="ipconfig"){
			choose(id,network[id]);
		}else if(id!="mac"){
			set(id,network[id]);
		}
	});
	//console.log("init complete");
}
function validate_mac(mac) {
	if(/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/.test(mac))
	{
		return(true);
	}
	return (false);
}
function validate_ip(ip)   
{  
	if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ip))  
	{  
		return (true)  
	}
	return (false)  
}
function check_value(obj){
	if(validate_ip(obj["ip"].value)==false){
		alert("Ошибочно введен IP адрес!\r\n Пример: 255.255.255.0 \n Состоять из 4-ех цифр от 0 до 255 разделенных точками.");
		return false;
	}
	if(validate_ip(obj["netmask"].value)==false){
		alert("Ошибочно введен адрес маски подсети!\r\n Пример: 255.255.255.0 \n Состоять из 4-ех цифр от 0 до 255 разделенных точками.");
		return false;
	}
	if(validate_ip(obj["gateway"].value)==false){
		alert("Ошибочно введен адрес шлюза!\r\n Пример: 192.168.1.1 \n Состоять из 4-ех цифр от 0 до 255 разделенных точками.");
		return false;
	}
	if(validate_mac(obj["mac"].value)==false){
		alert("Ошибочно введен MAC адрес!\r\n Пример: 0A:1B:2C:3D:4E:5F или 0A-1B-2C-3D-4E-5F. \n Состоять из цифр от 0 до 9 и букв A,B,C,D,E,F разделенных двоеточием или тире.");
		return false;
	}
	return true;
}
