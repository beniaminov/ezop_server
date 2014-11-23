function SendET() {
    document.getElementById("ExtTemplates").value = document.getElementById("field_for_ExtTempl").value;
    var k = document.getElementById("ExtTemplates").value;
    //alert(k);
    SendTempl_form.submit();
}

function getParam(name) {
    name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
    var regexS = "[\\?&]" + name + "=([^&#]*)";
    var regex = new RegExp(regexS);
    var results = regex.exec(window.location.href);
    if (results == null)
        return "";
    else
        return results[1];
}
function stringIsEmpty(StringToCheck) {
    if (!StringToCheck)
        return false;
    StringToCheck = StringToCheck.replace(/^\s+|\s+$/, '');
    if (StringToCheck.length == 0)
        return true;
    else
        return false;
}

function appendParamToFormIfExists(paramName, formID) {
    var paramVal = getParam(paramName);
    if (stringIsEmpty(paramVal)) 
        return;
    var hiddenField1 = document.createElement("input");
    hiddenField1.setAttribute("type", "hidden");
    hiddenField1.setAttribute("name", paramName);
    hiddenField1.setAttribute("value", paramVal);
    document.getElementById(formID).action = document.getElementById(formID).action + "?" + paramName + "=" + paramVal;

}


function GetCommand() {
    document.getElementById("curcnpt_text").value = document.getElementById("concept_textarea").value;
    //alert(document.getElementById("curcnpt_text").value );
    document.getElementById("rest").value = document.getElementById("concept_rest").value;
    //document.getElementById("cmd").value = document.getElementById("concept_cmd").value;
}

function NotNew() {
    if (document.getElementById("name_cnpt").innerText == "Новое понятие") { return false; }
    else { return true; }
}



/*
function GetNewCnptName (){
var retval = showModalDialog("newcnptname.html","Dialog Arguments Value","dialogHeight: 210px; dialogWidth: 250px;");
if (retval) {
document.getElementById("new_cnpt_name").value = retval; 
GetCommand();
return true;
}
else return false;
}
*/




function GetNewCnptName() {
    //var thename=window.prompt("Введите новое имя онтологии.","");	
    //if(thename) {

    if (document.getElementById("name_cnpt").value != "") {
        document.getElementById("new_cnpt_name").value = document.getElementById("name_cnpt").value;
        return true;
    }
    else {
        //alert("Ошибка: введите имя новой онтологии.");
        return false;
    }
}

function RenameOntology(event) {
    // if (Built()) {
    if (event.keyCode == 13) {
        if (GetNewCnptName() != false) {
            GetCommand();
            document.getElementById("curcnpt_name").value = document.getElementById("name_cnpt").value;

            document.getElementById("menu_item").value = "CC_rename";
            document.getElementById("inset").value = "RenameOntology";
            document.getElementById("version").value = document.getElementById("cnpt_ver").value;

            main_form.action = "editor.exe";
            main_form.target = "_self";
	    appendParamToFormIfExists("group", "main_form");
            main_form.submit();
        }
        else return;
    }
}





function Show_Ontology() {
    document.getElementById("menu_item").value = "CC_edit";
    document.getElementById("inset").value = "CC";
    main_form.action = "editor.exe";
    main_form.target = "_self";
    appendParamToFormIfExists("group", "main_form");
    main_form.submit();
}


function Edit_Ontology() {
    GetCommand();
    document.getElementById("menu_item").value = "CC_editNewVer";
    document.getElementById("mode").value = "editing";
    document.getElementById("inset").value = "CreateNewVersion";
    main_form.action = "editor.exe";
    main_form.target = "_self";
    main_form.submit();
}




function Run_Command() {
    // if (Built()) {
    if (document.getElementById("concept_cmd").value != "") {

        //GetCommand();

        document.getElementById("cmd").value = document.getElementById("concept_cmd").value;

        document.getElementById("menu_item").value = "CMD_run";

        document.getElementById("inset").value = "CC";

        main_form.action = "editor.exe";
        main_form.target = "_self";
	appendParamToFormIfExists("group", "main_form");
        main_form.submit();
    }
    else {
        alert("Команда не может быть пустой");
    }
    //}
    //else { 
    //          alert('Текст онтологии не построен!!! Постойте текст!.');
    //}
}

function Build_All() {
    if ((document.getElementById("concept_textarea").value + document.getElementById("concept_rest").value) != "") {

        if (document.getElementById("curcnpt_id").value == "") {
            //GetCommand();
            GetId("CC");
        }
        else {
            Save_Draft();
            
            document.getElementById("menu_item").value = "CC_build_all";
            document.getElementById("rest").value = "";
            document.getElementById("inset").value = "CC";
            document.getElementById("version").value = document.getElementById("cnpt_ver").value;

            main_form.action = "editor.exe";
            main_form.target = "_self";
            appendParamToFormIfExists("group", "main_form");
            main_form.submit();
        }
    }
    else {
        alert("Поле с текстом онтологии не может быть пустым");
    }
}


function Build_Complete() {
    if (Built()) {
        if (document.getElementById("concept_rest").value != "") {
            GetCommand();
            document.getElementById("menu_item").value = "CC_build_complete";
            document.getElementById("inset").value = "CC";
            main_form.action = "editor.exe";
            main_form.target = "_self";
	    appendParamToFormIfExists("group", "main_form");
            main_form.submit();
        }
        else {
            alert("Поле для необработанной части текста онтологии не может быть пустым!");
        }
    }
    else {
        alert('Текст онтологии не построен!!! Постойте текст!.');
    }
}




function Built() {
    var a = document.getElementById("concept_textarea").value;
    var b = document.getElementById("defin1").value;
    // alert(a);
    // alert(b);
    if (a == b) { return true; }
    else { return false; }
}


function SaveOntologyFinal() {
    if (Built()) {
        GetCommand();
        //document.getElementById("mode").value="";
        document.getElementById("menu_item").value = "CC_save";
        document.getElementById("inset").value = "SaveOntology";
        document.getElementById("version").value = document.getElementById("cnpt_ver").value;

        main_form.action = "editor.exe";
        main_form.target = "_self";
        appendParamToFormIfExists("group", "main_form");
        main_form.submit();
    }
    else {
        alert('Текст онтологии не построен!!! Постройте текст, затем сохраните.');
    }
}

function SaveOntologyFinalSRV() {
    if (Built()) {
        GetCommand();
        if (document.getElementById("version").value == "") {
            document.getElementById("menu_item").value = "CC_save";
        }
        else {
            document.getElementById("menu_item").value = "CC_editNewVer";
        }
        document.getElementById("ext_templ").value = document.getElementById("field_for_addedTempl").value;
        main_form.action = "../proc_data.php";
        main_form.target = "_blank";
        appendParamToFormIfExists("group", "main_form");
        main_form.submit();
    }
    else {
        alert('Текст онтологии не построен!!! Постройте текст, затем сохраните.');
    }
}





function SaveSRV() {
    if (document.getElementById("inset").value == "CreateNewVersion") {
        //alert('CrNewVer');
        GetCommand();
        document.getElementById("menu_item").value = "CC_editNewVer";
        main_form.action = "../proc_data.php";
        main_form.target = "_blank";
	appendParamToFormIfExists("group", "main_form");
        main_form.submit();
    }
    else if (document.getElementById("inset").value == "RenameOntology") {
        //alert('RenameOnt');
        GetCommand();
        document.getElementById("menu_item").value = "CC_rename";
        main_form.action = "../proc_data.php";
        main_form.target = "_blank";
	appendParamToFormIfExists("group", "main_form");
        main_form.submit();
    }
    else if (document.getElementById("inset").value == "SaveDraft") {
        //alert("draft"); 
        GetCommand();
        document.getElementById("menu_item").value = "SaveDraft";

        if (document.getElementById("rest").value != "") {
            Text = document.getElementById("curcnpt_text").value;
            Rest = document.getElementById("rest").value;
            document.getElementById("curcnpt_text").value = Text + Rest;
            document.getElementById("rest").value = "";
        }
        //alert(document.getElementById("curcnpt_text").value);
        document.getElementById("version").value = document.getElementById("cnpt_ver").value;
        main_form.action = "../proc_data.php";
        main_form.target = "_blank";
	appendParamToFormIfExists("group", "main_form");
        main_form.submit();
        //window.close();

    }
    else return;
}




function Set_Messages() {
    GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "MSG_set";
    main_form.action = "editor.exe";
    main_form.target = "_self";
    main_form.submit();
}



function Show_DIC_all_ext_templ() {
    //GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "DIC_all_ext_templ";
    main_form.action = "editor.exe";
    main_form.target = "_blank";
    main_form.submit();
}

function Show_DIC_all_cur_c() {
    GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "DIC_all_cur_c";
    main_form.action = "editor.exe";
    main_form.target = "_blank";
    main_form.submit();
}

function Show_DIC_all_cur_env() {
    GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "DIC_all_cur_env";
    main_form.action = "editor.exe";
    main_form.target = "_blank";
    main_form.submit();
}

function Show_DIC_rules() {
    GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "DIC_rules";
    main_form.action = "editor.exe";
    main_form.target = "_blank";
    main_form.submit();
}



//window.setInterval("SaveDraft()", 200000);

function Save_Draft() {

    if (document.getElementById("curcnpt_id").value != "") {

        //alert('id not empty!!!');
        // GetCommand();
        //document.getElementById("menu_item").value="SaveDraft";


        document.getElementById("inset").value = "SaveDraft";


        //Str=Text+"*&*"+Rest;
        //alert(document.getElementById("curcnpt_text").value);
        SaveSRV();

    }
    else {


        GetId("SaveOntology");

        //Save,call save_draft()
        //alert('!!!');
        //GetCommand();
        //document.getElementById("menu_item").value="CC_save";
        //document.getElementById("inset").value = "SaveOntology";

        //main_form.action="editor.exe";
        //main_form.target="_self";
        //main_form.submit();

        //Save_Draft
        //window.location.reload();
        //document.getElementById("inset").value = "SaveDraft";
        //SaveSRV();

        //window.setTimeout('alert(document.getElementById("curcnpt_id").value)', 3000);
        //clearTimeout(t);
        //alert(document.getElementById("curcnpt_id").value);	

    }

}

// window.onbeforeunload=start;

function GetId(Inset) {
    //alert('!!!');
    GetCommand();
    document.getElementById("menu_item").value = "CC_save";
    document.getElementById("inset").value = Inset;
    document.getElementById("curcnpt_name").value = document.getElementById("name_cnpt").value;
    //alert(document.getElementById("curcnpt_name").value );
    document.getElementById("version").value = document.getElementById("cnpt_ver").value;

    main_form.action = "editor.exe";
    main_form.target = "_self";
    appendParamToFormIfExists("group", "main_form");
    main_form.submit();


}

function start() {
    var myInt = setInterval("test()", 10);
    alert('close1!');

}


function checkLogin() {
    alert('AAAA check login!');
}

function test() {
    if (Window.closed == true) {
        clearInterval(myInt);
        checkLogin();
    }
}


function afterDel() {
    //alert('function afterdel is working!');

    SendAfterDel_form.submit();
    //window.close();
}

function New_Command() {
    //GetCommand();
    //document.getElementById("inset").value = "MSG";
    document.getElementById("menu_item").value = "new_command";
    main_form.action = "editor.exe";
    main_form.target = "_blank";
    main_form.submit();
}

function textChanged() {
    //alert("textChanged");
    //document.getElementById("state").style.display="none";
    document.getElementById("state").innerHTML = "";
    //window.location.reload();
}


function NewVersion() {
    document.getElementById("curcnpt_text").value = document.getElementById("concept_textarea").value;

    document.getElementById("menu_item").value = "CC_newver";
    document.getElementById("inset").value = "CC_newver";

    main_form.action = "editor.exe";
    main_form.target = "_self";
    main_form.submit();

}


function afterChangeVer() {
    window.close();


}