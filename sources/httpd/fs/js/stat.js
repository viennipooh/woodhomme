var stats = {
	<!--#system_stat-->
}
var stat_sel_val =[
	["RxLoPriorityByte","Вх.Низ.Приор.Байты"],
	["RxHiPriorityByte","Вх.Выс.Приор.Байты"],
	["RxUndersizePkt","Вх.Безразм.Пакеты"],
	["RxFragments","Вх.Фргменты"],
	["RxOversize","Вх.Превыш.Разм."],
	["RxJabbers","Вх.Джаббер"],
	["RxSymbolError","Вх.Ошиб.Симв."],
	["RxCRCerror","Вх.Ошиб.CRC"],
	["RxAlignmentError","Вх.Ошиб.Выравн."],
	["RxControl8808Pkts","Вх.Контр.8808-Пакеты"],
	["RxPausePkts","Вх.Пакеты.Паузы"],
	["RxBroadcast","Вх.Броадкастовые"],
	["RxMulticast","Вх.Мультикастовые"],
	["RxUnicast","Вх.Уникастовые"],
	["Rx64Octets","Вх.До64Байт"],
	["Rx65to127Octets","Вх.От65До127Байт"],
	["Rx128to255Octets","Вх.От128До255Байт"],
	["Rx256to511Octets","Вх.От256До511Байт"],
	["Rx512to1023Octets","Вх.От512До1023Байт"],
	["Rx1024to1522Octets","Вх.От1024До1522Байт"],
	["TxLoPriorityByte","Исх.Низ.Приор.Байты"],
	["TxHiPriorityByte","Исх.Выс.Приор.Байты"],
	["TxLateCollision","Исх.Поздн.Столкновения"],
	["TxPausePkts","Исх.Пакеты.Паузы"],
	["TxBroadcastPkts","Исх.Броадкастовые"],
	["TxMulticastPkts","Исх.Мультикастовые"],
	["TxUnicastPkts","Исх.Уникастовые"],
	["TxDeferred","Исх.Отложенные"],
	["TxTotalCollision","Исх.Всего.Столкновений"],
	["TxExcessiveCollision","Исх.Чрезмерн.Столкновений"],
	["TxSingleCollision","Исх.сОднимСтолкновением"],
	["TxMultipleCollision","Исх.сМнож.Столкновением"],
	["TxDropPackets","Исх.ОтброшенныеПакеты"],
	["RxDropPackets","Вх.ОтброшенныеПакеты"]
]
function window_onload(){
	for(i=0;i<5;i++)
	{
		var list=document.getElementsByName("stat_sel_"+(i+1));
		if(list){
			var j=0;stat_sel_val.forEach(function(item,i,arr){
				var option = document.createElement("option");
				option.text = ""+item[1];
				option.value = j++;
				list[0].add(option);
			});
		}
	}
	Object.keys(stats).forEach(function(id){
		if(id=="stat_sel_1" || id=="stat_sel_2" || id=="stat_sel_3" || id=="stat_sel_4" || id=="stat_sel_5"){
			var list=document.getElementsByName(id);
			list[0].selectedIndex = stats[id];
		}else if(id=="stat_1_" || id=="stat_2_" || id=="stat_3_" || id=="stat_4_"){
			var i=0;Object.keys(stats[id]).forEach(function(it){
				++i;
				if(document.getElementById(id+i)){
					document.getElementById(id+i).innerHTML = stats[id][it];
				}
			});
		}
	});
}