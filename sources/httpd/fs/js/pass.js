
function validate_string(str)   
{
	if(str.length > 16)
	{
		return (false)
	}
	if(str.length < 4)
	{
		return (false)
	}
	if (/^[_a-zA-Z0-9]+$/.test(str))  
	{
		return (true)  
	}
	return (false)  
}

function check_value(obj){
	if(validate_string(obj["login"].value)==false){
		alert("Логин должен содержать буквы латинского алфавита, цифры и символ нижнего подчеркивания. (a-z;A-Z;0-9;_) !\r\n");
		return false;
	}
	if(validate_string(obj["pass"].value)==false){
		alert("Пароль должен содержать буквы латинского алфавита, цифры и символ нижнего подчеркивания. (a-z;A-Z;0-9;_) !\r\n");
		return false;
	}
	if(validate_string(obj["newlogin"].value)==false){
		alert("Новый логин должен содержать буквы латинского алфавита, цифры и символ нижнего подчеркивания. Длина логина не может превышать 16 символов и быть меньше 4-ех. (a-z;A-Z;0-9;_) !\r\n");
		return false;
	}
	if(validate_string(obj["newpass"].value)==false){
		alert("Новый пароль должен содержать буквы латинского алфавита, цифры и символ нижнего подчеркивания. Длина пароля не может превышать 16 символов и быть меньше 4-ех. (a-z;A-Z;0-9;_) !\r\n");
		return false;
	}
	if(obj["newpass2"].value != obj["newpass"].value){
		alert("Новые пароли не совпадают! А должны.\r\n");
		return false;
	}
	return true;
}
