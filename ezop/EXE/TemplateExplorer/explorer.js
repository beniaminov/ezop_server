/*
function fnChange(){
   if (templatesList.options[templatesList.selectedIndex].value!=""){
   exp_concept_text.value=templatesList.options[templatesList.selectedIndex].value;
   }
   else { 
   exp_concept_text.value="Описание для этого шаблона ещё не определено.";
   }  
}
*/





function fnChange (){
alert("!!!");
    document.getElementById('explorer_selected_left').value = document.getElementById('tmpl_select_left').options[document.getElementById('tmpl_select_left').selectedIndex].value;
	main_form.submit();
}



/*
function OnClick(){
window.parent.document.getElementById("menu_item").value = "toolbar";
window.parent.document.getElementById("toolbar_button").value = id;
window.parent.document.getElementById("main_form").target = "content";
main_form.submit();
}
*/






function ChangeSelection (SelectedValue, IDValue){
	//alert(SelectedValue);
 //if (SelectedValue == "   [..]") DisableButtonsUp();
 window.parent.document.getElementById(IDValue).value = SelectedValue;
}

function ifDisabled() {
if (fldr_left_path.innerText == "Список понятий: весь") DisableButtons();
}

function DisableButtons(){
	fldr_left_create.disabled = "true";
	fldr_left_fromfolder.disabled = "true";
	fldr_left_replace.disabled = "true";
}

function DisplayBottom() {
var Name;
//window.parent.document.getElementById(explorer_selected_left).value="-+@index+";
//Name = window.parent.document.getElementById(explorer_selected_left).value;
Name = "@New_Область1 - это @Область2";
//alert(Name);
res = checkName (Name);
//alert(res);

//if (window.document.getElementById("tmpl_select_left").selectedIndex!=-1 && res==-1) {
//	window.parent.document.getElementById("menu_item").value = "toolbar";
//	window.parent.document.getElementById("toolbar_button").value = "tmpl_left_display";
//	window.parent.document.getElementById("main_form").target = "exp_concept_text";
	main_form.submit();
//	}
//else {window.open("about:blank", "exp_concept_text");}
}

function checkName (Name){
var s,  rr, boolres;
	s = Name;	
	//rr = s.search (/[[]]/);
	rr = s.search (/[[..]]/);
return (rr);
}

/*
function DisplayBlank(){
var Name;
Name = window.parent.document.getElementById(explorer_selected_left).value;
res = checkName (Name);

if (window.document.getElementById("tmpl_select_left").selectedIndex!=-1 && res==-1) {
	window.parent.document.getElementById("menu_item").value = "toolbar";
	window.parent.document.getElementById("toolbar_button").value = "fldr_left_display";
	window.parent.document.getElementById("main_form").target = "_blank";
	main_form.submit();
	}
}
*/