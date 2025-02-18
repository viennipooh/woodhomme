<!--
var iperrmsg = " is not a valid IP address."
function CheckValue(form)
{
if( !verifyIP(form.IP.value) )
{
alert(form.IP.value + iperrmsg);
form.IP.focus();
return false;
}
if( form.Netmask.value.length > 0 )
{
if( !verifyNetmask(form.Netmask.value) )
{
alert(form.Netmask.value + iperrmsg);
form.Netmask.focus();
return false;
}
}
if( form.Gateway.value.length > 0 )
{
if( !verifyNetmask(form.Gateway.value) )
{
alert(form.Gateway.value + iperrmsg);
form.Gateway.focus();
return false;
}
}
if( form.DNS1.value.length > 0 )
{
if( !verifyNetmask(form.DNS1.value) )
{
alert(form.DNS1.value + iperrmsg);
form.DNS1.focus();
return false;
}
}
if( form.DNS2.value.length > 0 )
{
if( !verifyNetmask(form.DNS2.value) )
{
alert(form.DNS2.value + iperrmsg);
form.DNS2.focus();
return false;
}
}
if( !isValidPort(form.IPLocationPort.value) )
{
alert(form.IPLocationPort.value + " is not a valid number");
form.IPLocationPort.focus();
return false;
}
if( form.IPLocationPeriod.value < 0 || !isValidNumber(form.IPLocationPeriod.value) )
{
alert(form.IPLocationPeriod.value + " is not a valid number");
form.IPLocationPeriod.focus();
return false;
}
return true;
}
//-->