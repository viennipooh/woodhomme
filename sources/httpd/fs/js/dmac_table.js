var dynamic_mac_table=[
<!--#system_dmac-->
];

var dynamic_mac_table_test=[
["70:B3:D5:52:00:5A","CPU","0","-"],["70:B3:D5:52:00:5A","CPU","0","-"],["00:15:5D:00:DC:02","7","0","-"],["00:50:FC:C7:F4:A7","6","0","-"],["10:BF:48:4A:98:F0","7","0","-"],["2C:D0:5A:A6:EF:C2","7","0","-"],["30:0D:43:24:91:B4","7","0","-"],["4C:CC:6A:AF:C4:05","7","0","-"],["60:E3:27:15:94:91","7","0","-"],["9C:5C:8E:76:E7:33","7","0","-"],["C8:BE:19:B6:21:B8","7","0","-"],
];
function set(sf,sv){document.network[sf].value=sv;}
function choose(sf,sv){document.network[sf].selectedIndex=sv;}
function window_onload(){
	var table=document.getElementById("dmac_table");
	table_item=document.createElement("TR");
	table_item.id = "dmac_header";
	table_item_in=document.createElement("TD");
	table_item_in.innerHTML="Индекс";//table_item_in
	table_item.appendChild(table_item_in);
	
	table_item_in=document.createElement("TD");
	table_item_in.innerHTML="MAC адрес";
	table_item.appendChild(table_item_in);
	
	table_item_in=document.createElement("TD");
	table_item_in.innerHTML="Номер порта";
	table_item.appendChild(table_item_in);
	
	table_item_in=document.createElement("TD");
	table_item_in.innerHTML="VLAN";
	table_item.appendChild(table_item_in);
	
	table_item_in=document.createElement("TD");
	table_item_in.innerHTML="VLAN ссылка";
	table_item.appendChild(table_item_in);
	
	table.appendChild(table_item);
	dynamic_mac_table.forEach(function(item,index,arr){
		table_item=document.createElement("TR");
		
		table_item_in=document.createElement("TD");
		table_item_in.innerHTML=index+1;//table_item_in
		table_item.appendChild(table_item_in);
		
		table_item_in=document.createElement("TD");
		table_item_in.innerHTML=item[0];
		table_item.appendChild(table_item_in);
		
		table_item_in=document.createElement("TD");
		table_item_in.innerHTML=item[1];
		table_item.appendChild(table_item_in);
		
		table_item_in=document.createElement("TD");
		table_item_in.innerHTML=item[2];
		table_item.appendChild(table_item_in);
		
		table_item_in=document.createElement("TD");
		table_item_in.innerHTML=item[3];
		table_item.appendChild(table_item_in);
		
		table.appendChild(table_item);
	});
}

