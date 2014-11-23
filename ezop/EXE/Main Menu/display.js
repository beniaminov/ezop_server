var SelectedID, LastSelection;
LastSelection = SelectedID;
var Content=window.parent.content;

function clearSubMTable(){
	subMenu.outerHTML = "<TABLE id=subMenu border=0 name=\"subMenu\"><TBODY></TBODY></TABLE>";

}
function ChangeBG_white_byID (ID) {
	window.document.getElementById(ID).runtimeStyle.backgroundColor = "white";}

function ChangeBG_white_byID_all () {
Ids = new Array();
Ids[0] = "FL"; Ids[1] = "KB"; Ids[2] = "CC"; Ids[3] = "CMD"; Ids[4] = "DIC"; Ids[5] = "MSG"; Ids[6] = "HLP";
var i = 0;
while (i<7)
	{
	ChangeBG_white_byID (Ids[i]);
	i++;
	}
} 

function ReturnToSelected(){
	//alert(LastSelection);
	if(LastSelection && LastSelection!=SelectedID) ChangeBG_white_byID(LastSelection);
	if(!SelectedID)	clearSubMTable();
	else {
		window.document.getElementById(SelectedID).runtimeStyle.backgroundColor = "CCCC99";
		DisplaySubMenu(SelectedID);
	}
	LastSelection="";
}
function clearTable(Table){
	Table.outerHTML = "<TABLE id=subMenu border=0 name=\"subMenu\"><TBODY></TBODY></TABLE>";
	Table = subMenu;
	var clearTable = Table;
	return clearTable;
}

function DisplayNodes(Node, tr)
{
	
	if(Node.nodeType == 1&& Node.tagName == "sub_menu_item") // тип узла 1 - элемент
	{
		td = tr.insertCell();   // добавл€ем €чейку в строку
		var text = "";
		text += "<td width=\"10\" ><B" + " ";
		// јттрибуты со значени€ми (если есть)
		for(var i = 0; i < Node.attributes.length; ++i)
		 if(Node.attributes(i).name !="value")
		   text += " " + Node.attributes(i).name + "=\"" + Node.attributes(i).value + "\"";
		 else {text += ">"+" " +Node.attributes(i).value+"</B>";
			break;}
		td.innerHTML += text;
	}
	//alert(tr.innerHTML);
	//alert(subMenu.outerHTML);
	
	
}

function DisplaySubMenu (MenuItemID){
	// если oNodeList(i) отвечает условию (подменю конкретного пункта меню, то Display Nodes дл€ всех его потомков)
	var xmlDoc = new ActiveXObject("Microsoft.XMLDOM");
	xmlDoc.async = false;
	xmlDoc.load("menu.xml");
	var root = xmlDoc.documentElement;
	var Table = subMenu;
	oNodeList = root.childNodes;
	for(var i = 0; i < oNodeList.length; ++i){
		//ADD IF-CONDITION
		if(oNodeList(i).attributes(0).value == MenuItemID){
		Table = clearTable(Table);
		var tr = Table.insertRow();
		tr.runtimeStyle.backgroundColor ="336699"; 
		tr.runtimeStyle.height = "35";
		for(var j = 0; j < oNodeList(i).childNodes.length; ++j)
			{DisplayNodes(oNodeList(i).childNodes(j),  tr);}//END INT FOR
		}//END IF
		}//END EXT FOR

}
