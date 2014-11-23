function OnClick(){
window.parent.document.getElementById("menu_item").value = "toolbar";
window.parent.document.getElementById("toolbar_button").value = id;
window.parent.document.getElementById("main_form").target = "content";
window.parent.document.getElementById("button").fireEvent("onclick");
}

//function ChangeSelectionL(SelectedValue){
//ChangeSelection(SelectedValue, "explorer_selected_left");
//}

//function ChangeSelectionR(SelectedValue){
//ChangeSelection(SelectedValue, "explorer_selected_right");
//}

//function VerifSelectedValue(SelectedValue) {
//alert(SelectedValue);
//if (SelectedValue == "   [..]"){//alert("dss"); DisableButtons();return false;}
//else {return true;}
//}

function ChangeSelection (SelectedValue, IDValue){
	//alert(SelectedValue);
 //if (SelectedValue == "   [..]") DisableButtonsUp();
 window.parent.document.getElementById(IDValue).value = SelectedValue;
}
function DblLeft() {
window.parent.document.getElementById("main_form").target = "content";
window.parent.document.getElementById("menu_item").value = "toolbar";
window.parent.document.getElementById("toolbar_button").value = "fldr_left_dblclick";
window.parent.document.getElementById("button").fireEvent("onclick");
}

function DblRight() {
window.parent.document.getElementById("main_form").target = "content";
window.parent.document.getElementById("menu_item").value = "toolbar";
window.parent.document.getElementById("toolbar_button").value = "fldr_right_dblclick";
window.parent.document.getElementById("button").fireEvent("onclick");
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
Name = window.parent.document.all.explorer_selected_left.value;
res = checkName (Name);


if (window.document.getElementById("select_left").selectedIndex!=-1 && res==-1) {
	window.parent.document.getElementById("menu_item").value = "toolbar";
	window.parent.document.getElementById("toolbar_button").value = "fldr_left_display";
	window.parent.document.getElementById("main_form").target = "exp_concept_text";
	window.parent.document.getElementById("button").fireEvent("onclick");
	}
else {window.open("about:blank", "exp_concept_text");}
}
function checkName (Name){
var s,  rr, boolres;
	s = Name;	
	//rr = s.search (/[[]]/);
	rr = s.search (/[[..]]/);

return (rr);
 
}


function DisplayBlank(){
var Name;
Name = window.parent.document.all.explorer_selected_left.value;
res = checkName (Name);


if (window.document.getElementById("select_left").selectedIndex!=-1 && res==-1) {
	window.parent.document.getElementById("menu_item").value = "toolbar";
	window.parent.document.getElementById("toolbar_button").value = "fldr_left_display";
	window.parent.document.getElementById("main_form").target = "_blank";
	window.parent.document.getElementById("button").fireEvent("onclick");
	}
}

function CCInset() {
	window.parent.document.getElementById("inset").value = "CC";

}