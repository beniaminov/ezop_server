function GetState() {
var ReturnString, i;
ReturnString = "";
i = 0;
checkBox = new Array ();
checkBox [0] = "ch1";
checkBox [1] = "ch2";
checkBox [2] = "ch3";
checkBox [3] = "ch4";
checkBox [4] = "ch5";
checkBox [5] = "ch6";
checkBox [6] = "ch7";
checkBox [7] = "ch8";

while (i<8) {
	if (document.getElementById(checkBox[i]).checked == true)	ReturnString+="1";
	else	{ReturnString+="0";}
	i ++;
	}


//alert (ReturnString);
            document.getElementById("msg_to_display").value = ReturnString ;
            document.getElementById("menu_item").value="CC_edit";
            //window.document.getElementById("inset").value = "CC";
            main_form.action="editor.exe";
            main_form.submit();
}