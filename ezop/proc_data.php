<?php
error_reporting(0);
//  Не помещать НпКАКОЙ вывод ДО этих строк, иначе будет ошибка заголовков!
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

// Объявления функций

function getCurrWebDir() {
 $pageURL = 'http';
 if ($_SERVER["HTTPS"] == "on") {$pageURL .= "s";}
 $pageURL .= "://";
 if ($_SERVER["SERVER_PORT"] != "80") {
  $pageURL .= $_SERVER["SERVER_NAME"].":".$_SERVER["SERVER_PORT"].$_SERVER["REQUEST_URI"];
 } else {
  $pageURL .= $_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"];
 }
 return dirname($pageURL). "/";
}

// Функция удаления группы
// $groupID - id группы
function FnDelGroup($groupID)
{
      $sql = "DELETE FROM {og} WHERE nid = %d";
      db_query($sql, $groupID);
      $sql = "DELETE FROM {og_ancestry} WHERE nid = %d";
      db_query($sql, $groupID);
      $sql = "DELETE FROM {og_uid} WHERE nid = %d";
      db_query($sql, $groupID); 
      $sql = "DELETE FROM {node} WHERE type = 'og' and nid = %d";
      db_query($sql, $groupID); 


}

// Функция сохранения онтологии в группе
// $newOntId - id онтологии
// $groupID - id группы
function FnAddOntToGroup($newOntId, $groupID)
{
 	db_query("INSERT INTO og_ancestry(nid, group_nid, is_public) values (".$newOntId.", ".$groupID.", 0)");
}

// Функция сохранения черновика на сервере
// $node_title - название онтологии
// $node_text - текст онтологии
// добавить список используемых онтологий, шаблонов и т.д. в отд. таблицы
function FnSaveDraft($curcnpt_id, $node_title, $node_text, $node_env, $node_used, $version)
{
	// Отладка
	//echo "FnSaveDraft";
	//echo "Author: ".$user->name;

	// Перекодировка в Юникод
	$node_title_uni = iconv('Windows-1251', 'UTF-8', $node_title);
	$node_text_uni = iconv('Windows-1251', 'UTF-8', $node_text);

	// Ищем Id Drupal по Id Prolog
	$ont_id_tmp = db_query("SELECT nid FROM draft_info WHERE field_nid_prolog = %s", $curcnpt_id);
	$ont_id_drup = db_result($ont_id_tmp);

	if($ont_id_drup>0)
	{
		//Обновляем таблицы
		db_query("UPDATE node_revisions SET body = '".mysql_real_escape_string($node_text_uni)."' WHERE nid = ".$ont_id_drup);
		db_query("UPDATE node SET title = '".mysql_real_escape_string($node_title_uni)."' WHERE nid = ".$ont_id_drup);
		return $ont_id_drup;
	}
	else
	{
		$draft_nid = make_new_node('draft', 1, 1, 0, $node_title_uni, $node_text_uni, '');

		// Превращаем строку используемых ID в массив
		$used_arr1 = substr($node_used, 1);
		$used_arr2 = str_replace("]", "", $used_arr1);

		//echo "<br>Arr: ".$used_arr;
		//echo "<br>Arr: ".$used_arr2;

		$u_arr = explode(",", $used_arr2);
		$used_ontology = 3;
		// Вставка отношений использования
		//foreach($u_arr as $used_ontology)
		//{
			//echo "<br>Used ont: ".$used_ontology;
			db_query("INSERT INTO draft_info
				(nid, field_id__drupal_value, field_parent_id_value, field_nid_prolog, version)
				values (".$draft_nid.", ".$node_env.", ".$used_ontology.", ".$curcnpt_id.", 3)");
		//}
		echo "Черновик сохранен. Страница будет закрыта";		return $draft_nid;
	}

}

// Переименование существующей онтологии
// $ont_id - id онтологии Пролог для переименования
// $new_name - новое имя онтологии
function FnRenameOntology($ont_id, $new_name)
{
	// Перекодируем новое имя в Юникод
	$new_name_uni = iconv('Windows-1251', 'UTF-8', $new_name);

	// пщем Id Drupal по Id Prolog
	$ont_id_tmp = db_query("SELECT field_id_drupal_value FROM content_type_id_match WHERE field_nid_prolog = %s", $ont_id);
	$ont_id_drup = db_result($ont_id_tmp);

	// Обновляем таблицу, используя полученное (просмотр онтологии берет данные из Node_revisions)
	db_query("UPDATE node_revisions SET title = '".mysql_real_escape_string($new_name_uni)."' WHERE nid = ".$ont_id_drup);

	// Обновляем таблицу Nodes (данные для списка Views берут оттуда)
	db_query("UPDATE node SET title = '".mysql_real_escape_string($new_name_uni)."' WHERE nid = ".$ont_id_drup);
}

// Создание новой версии при переходе в режим редактирования
// $parent_dr_id - id prolog "старой" онтологии
// $child_dr_id - id "новой"
// остальные параметры аналогичны FnMakeNewOnt
function FnSaveVersion($parent_dr_id, $child_dr_id, $ont_name, $ont_text, $ont_env_id, $used_arr, $version)
{
	// Добавление в таблицу версий (участвуют ID Пролог с обеих сторон!!)
	$nid_ver = db_next_id(nid);
	$vid_ver = db_next_id(vid);

	db_query("INSERT INTO content_type_version
	(nid, vid, field_parent_id_0_value, field_child_id_0_value)
	values (".$nid_ver.", ".$vid_ver.", ".$parent_dr_id.", ".$child_dr_id.")");

	// Добавление новой онтологии в БД (версия - это самостоятельная новая онтология!)
	FnMakeNewOnt($child_dr_id, $ont_name, $ont_text, $ont_env_id, $used_arr, $version);

	//echo "New Version Saved!";
}


// Создание новой онтологии
// $id_ontology - id онтологии в Прологе
// $ont_name - название онтологии
// $ont_text - текст онтологии
// $ont_env_id - id среды
function FnMakeNewOnt($id_ontology, $ont_name, $ont_text, $ont_env_id, $used_arr, $version)
{
	// Необходима перекодировка, т.к. кодировка Пролог - windows-1251, кодировка Друпал - UTF-8
	$ont_name_uni = iconv('Windows-1251', 'UTF-8', $ont_name);
	$ont_text_uni = iconv('Windows-1251', 'UTF-8', $ont_text);

	// Отладочный вывод, как отпадет необходимость - убрать
	//echo "<br>Inside making Fn...";
	//echo "<br>id_ont после перекодировки: ".$id_ontology;
	//echo "<br>ont_name: ".$ont_name_uni;
	//echo "<br>text: ".$ont_text_uni;
	//echo "<br>env_id: ".$ont_env_id;

	// Создаем новую онтологию
	$ont_nid = make_new_node('content_', 1, 1, 0, $ont_name_uni, $ont_text_uni, '');

	// Находим версию (vid)
	$ont_vid_tmp = db_query("SELECT vid FROM `node_revisions` WHERE nid = %s", $ont_nid);
	$ont_vid = db_result($ont_vid_tmp);

	// Отладка
	//echo "<br>VID: ".$ont_vid;

	// Добавляем соответствие id онтологии друпала и id онтологии в прологе
	$nid_match = db_next_id(nid);
	$vid_match = db_next_id(vid);

	db_query("INSERT INTO content_type_id_match
	(nid, vid, field_nid_prolog, field_id_drupal_value)
	values (".$nid_match.", ".$vid_match.", ".$id_ontology.", ".$ont_nid.")");

	// Добавляем отношение "Быть средой"
	$nid_env = db_next_id(nid);
	$vid_env = db_next_id(vid);

	db_query("INSERT INTO content_type_environment
	(nid, vid, field_id__drupal_value, field_id____value)
	VALUES (%s, %s, %s, %s)", $nid_env, $vid_env, $ont_env_id, $id_ontology);

	// Вставка в таблицу CCK (content_type_content_), чтобы отображалось еще и красиво
	$href = "<a href=?q=Kernel>Ядро системы</a>";
	if($ont_env_id != 1)
	{
		$env_info = db_fetch_object(db_query("SELECT node.nid, node.title FROM node, content_type_id_match WHERE content_type_id_match.field_nid_prolog = ".$ont_env_id." and content_type_id_match.field_id_drupal_value = node.nid LIMIT 1"));
		if(!empty($env_info))
			$href = "<a href='?q=node/".$env_info->nid."'>".$env_info->title."</a>";
	}
	$no_uses = "В данной онтологии отношение использования не установлено";
	$no_templates = "В данной онтологии шаблоны не разработаны";
	db_query("INSERT INTO content_type_content_
			(version_number,concept_id, nid, vid, field___value, field___format, field___0_value,
			field___1_value, field___1_format, field___0_format, field______value, field______format)
			VALUES
			('".$version."', 1, '".$ont_nid."', '".$ont_vid."', '".mysql_real_escape_string($href)."', 1, '".mysql_real_escape_string($ont_text_uni)."', '".$no_uses."', 1, 1, '".$no_templates."', 1)");
	//db_query("DELETE FROM node_revision WHERE title = '%s'", $ont_name);

	// Превращаем строку используемых ID в массив
	$used_arr1 = substr($used_arr, 1);//отделили первую скобку
	$used_arr2 = str_replace("]", "", $used_arr1);//заменили последнюю скобку на пустой символ

	//echo "<br>Arr: ".$used_arr;
	//echo "<br>Arr: ".$used_arr2;

	$u_arr = explode(",", $used_arr2);//разбиваем строку на массив о разделителю - ,

	// Вставка отношений использования
	foreach($u_arr as $used_ontology)
	{
		echo "<br>Used ont: ".$used_ontology;
		$vid_use = db_next_id(vid);
		$nid_use = db_next_id(nid);

		db_query("INSERT INTO content_type_using
				 (vid, nid, field_parent_id_value, field_child_id_value)
				 VALUES
				 (".$vid_use.", ".$nid_use.", ".$used_ontology.", ".$id_ontology.")");
	}

	//foreach($u_arr as $used_ontology)
	//{
	//	echo "<br>Used ont: ".$used_ontology;

	//	$ont_id_tmp = db_query("SELECT field_parent_id_value FROM content_type_using WHERE field_child_id_value = %s", $used_ontology);
	//	$ont_id = db_result($ont_id_tmp);
	//	$ont_name_tmp = db_query("SELECT title FROM node WHERE nid = %s", $ont_id);
	//	$ont_name = db_result($ont_name_tmp);

	//	echo "<br>Name ont: ".$ont_name;

	//}

	// Ищем Id Drupalчерновика по IdOntology Prolog
    	$ont_id_tmp = db_query("SELECT nid FROM draft_info WHERE field_nid_prolog = %s", $id_ontology);
    	$ont_id_drup = db_result($ont_id_tmp);

   	 // Удаление черновика в БД
	db_query("DELETE FROM node WHERE nid = %s", $ont_id_drup);
	db_query("DELETE FROM node_revisions WHERE nid = %s", $ont_id_drup);
	db_query("DELETE FROM draft_info WHERE nid = %s", $ont_id_drup);
	
	return $ont_nid;
}


//Удаление онтологий
//$old_id - id в прологе
//$ont_id_drup - id в друпале
function FnDelOnt($ont_id_drup,$ont_id_prolog)
{	
	global $user;
	$returnURL = "?q=user/". $user->uid;
	$group_nid = db_result(db_query("SELECT group_nid FROM og_ancestry WHERE nid = ".$ont_id_drup));
	if($group_nid > 0)
		$returnURL = "?q=node/" . $group_nid;

	$returnURL = getCurrWebDir() . $returnURL;

	try 
	{		

	   	 // Ищем Id Drupal по Id Prolog 
		if($ont_id_drup == "")
			$ont_id_drup = db_result(db_query("SELECT field_id_drupal_value FROM content_type_id_match WHERE field_nid_prolog=".$ont_id_prolog." LIMIT 1"));
		if($ont_id_prolog == "")
			$ont_id_prolog = db_result(db_query("SELECT field_nid_prolog FROM content_type_id_match WHERE field_id_drupal_value=".$ont_id_drup." LIMIT 1"));

		if($ont_id_drup == "")
			$ont_id_drup = 0;
		if($ont_id_prolog == "")
			$ont_id_prolog = 0;

		// Удаляем из всех таблиц
	    	if (!mysql_query("DELETE FROM content_type_id_match WHERE field_id_drupal_value = ".$ont_id_drup)) 
			 throw new Exception("Query failed (content_type_id_match): (".$ont_id_drup.") " . mysql_error());
	    	if (!mysql_query("DELETE FROM node_revisions WHERE nid = ".   $ont_id_drup)) 
			 throw new Exception("Query failed (node_revisions): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM content_type_environment WHERE field_id____value = ".   $ont_id_prolog)) 
			 throw new Exception("Query failed (content_type_environment): (".$ont_id_prolog.") " . mysql_error());
	    	if (!mysql_query("DELETE FROM content_type_environment WHERE field_id__drupal_value = ".   $ont_id_prolog)) 
			 throw new Exception("Query failed (content_type_environment): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM content_type_content_ WHERE nid = ".   $ont_id_drup)) 
			 throw new Exception("Query failed (content_type_content_): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM content_type_using WHERE field_parent_id_value  = ".   $ont_id_prolog)) 
			 throw new Exception("Query failed (content_type_using): " . mysql_error());
	    	if (!mysql_query("DELETE FROM content_type_using WHERE field_child_id_value  = ".   $ont_id_drup)) 
			 throw new Exception("Query failed (content_type_using): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM content_type_version WHERE field_child_id_0_value  = ".   $ont_id_prolog)) 
			 throw new Exception("Query failed (content_type_version): " . mysql_error());
	    	if (!mysql_query("DELETE FROM content_type_version WHERE field_parent_id_0_value  = ".   $ont_id_prolog)) 
			 throw new Exception("Query failed (content_type_version): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM og_ancestry WHERE nid  = ".   $ont_id_drup)) 
			 throw new Exception("Query failed (og_ancestry): " . mysql_error());
	
	    	if (!mysql_query("DELETE FROM node WHERE nid  = ".   $ont_id_drup)) 
			 throw new Exception("Query failed (node): " . mysql_error());

		if(!($ont_id_drup == 0 && $ont_id_prolog == 0))
			print "<h1>Онтология успешно удалена</h1><br/>";
		else
		{
			print "<h1>Онтология удалена с ошибками:</h1><br/>";
			if($ont_id_drup == 0)
				print "<p>Не найден идентификатор онтологии в Drupal</p><br/>";
			if($ont_id_prolog == 0)
				print "<p>Не найден идентификатор онтологии в Prolog</p><br/>";
		}



	}
	catch (Exception $e) 
	{
		print "<h1>Ошибка при удалении онтологии:</h1>";
		print "<p>" . $e->getMessage()."</p><br/>";
		print "<a href='".getCurrWebDir() . "?q=node/" . $ont_id_drup."'>Вернуться назад</a>";
	}

	print "<a href='".$returnURL."'>Продолжить работу в системе</a>";

}

function FnDelDr($ont_id_drup)
{
	global $user;
	$returnURL = "?q=user/". $user->uid;
	$group_nid = db_result(db_query("SELECT group_nid FROM og_ancestry WHERE nid = ".$ont_id_drup));
	if($group_nid > 0)
		$returnURL = "?q=node/" . $group_nid;

	$returnURL = getCurrWebDir() . $returnURL;

	try 
	{		
	   	 // Удаление черновика в БД
		if (!mysql_query("DELETE FROM node WHERE nid = ". $ont_id_drup)) 
			 throw new Exception("Query failed (node): " . mysql_error());
		if (!mysql_query("DELETE FROM node_revisions WHERE nid = ". $ont_id_drup)) 
			 throw new Exception("Query failed (node_revisions): " . mysql_error());
		if (!mysql_query("DELETE FROM draft_info WHERE nid = ". $ont_id_drup)) 
			 throw new Exception("Query failed (draft_info): " . mysql_error());

		print "<h1>Черновик успешно удален</h1><br/>";

	}
	catch (Exception $e) 
	{
		print "<h1>Ошибка при удалении черновика:</h1>";
		print "<p>" . $e->getMessage()."</p><br/>";
		print "<a href='".getCurrWebDir() . "?q=node/" . $ont_id_drup."'>Вернуться назад</a>";
	}

	print "<a href='".$returnURL."'>Продолжить работу в системе</a>";
}

function FnMakeMainVer($curcnpt_id, $version)
{
	 // Ищем Id Drupal по Id Prolog
    	$ont_id_tmp = db_query("SELECT field_id_drupal_value FROM content_type_id_match WHERE field_nid_prolog = %s", $curcnpt_id);
    	$ont_id_drup = db_result($ont_id_tmp);

	//Добавляем в таблицу главных версий
	db_query("INSERT INTO content_version
	(nid, nid_prolog, main_version)
	values (".$ont_id_drup.", ".$curcnpt_id.", ".$version.")");

}

function FnChMainVer($curcnpt_id, $main_id)
{
	 // Ищем Id Drupal по Id Prolog
    	$ont_id_tmp = db_query("SELECT field_id_drupal_value FROM content_type_id_match WHERE field_nid_prolog = %s", $curcnpt_id);
    	$ont_id_drup = db_result($ont_id_tmp);

	//Удаляем старую главную версию
	db_query("DELETE FROM content_version WHERE nid = %s", $ont_id_drup);

	//Записываем новую главную версию
	db_query("INSERT INTO content_version
	(nid, nid_prolog, main_version)
	values (".$ont_id_drup.", ".$curcnpt_id.", 1)");
}


// Теперь можно выводить

// Анализируем тип действия
$action = $_POST['menu_item'];

// В зависимости от типа действия выполняем те или иные действия
if ($action == "SaveDraft")
{
	//Создана ли онтология в какой-либо группе
	$groupID = 0;
	if (isset($_REQUEST['group']))
	   if(!empty($_REQUEST['group']))
		 $groupID = $_REQUEST['group'];

	$newOntId = FnSaveDraft($_POST['curcnpt_id'], $_POST['curcnpt_name'], $_POST['curcnpt_text'], $_POST['env_id'], $_POST['usedIds'], $_POST['version']);
	//echo "Draft saved...";
	
	if($groupID != 0)
		FnAddOntToGroup($newOntId, $groupID);
	echo "<h1>Mthd:savedraft, GrID:".$groupID."</h1>";
}

if ($action == "CC_build_all")
{	$groupID = 0;
	if (isset($_REQUEST['group']))
	   if(!empty($_REQUEST['group']))
		 $groupID = $_REQUEST['group'];
	$newOntId = FnSaveDraft($_POST['curcnpt_id'], $_POST['curcnpt_name'], $_POST['curcnpt_text'], $_POST['env_id'], $_POST['usedIds'], $_POST['version']);
	//echo "Draft saved...";	if($groupID != 0)
		FnAddOntToGroup($newOntId, $groupID);
	echo "<h1>Mthd:buildall, GrID:".$groupID."</h1>";
}


// Сохранение онтологии по "Сохранить онтологию и завершить редактирование"
else if ($action == "CC_save")
{
	echo "CC_Save (saving ontology) working...";

	//Создана ли онтология в какой-либо группе
	$groupID = 0;
	if (isset($_REQUEST['group']))
	   if(!empty($_REQUEST['group']))
		 $groupID = $_REQUEST['group'];

	$newOntId = FnMakeNewOnt($_POST['curcnpt_id'], $_POST['curcnpt_name'], $_POST['curcnpt_text'], $_POST['env_id'], $_POST['usedIds'],$_POST['version']);
	if($groupID != 0)
		FnAddOntToGroup($newOntId, $groupID);
}

else if ($action == "CC_rename")
{
	//echo "Rename working";
	FnRenameOntology($_POST['curcnpt_id'], $_POST['curcnpt_name']);
}

else if($action == "CC_editNewVer")
{
	echo "CC_editNewVer";
	FnSaveVersion($_POST['old_id'], $_POST['curcnpt_id'], $_POST['curcnpt_name'], $_POST['curcnpt_text'], $_POST['env_id'], $_POST['usedIds'], $_POST['version']);
}

else if ($action == "delete_ontology")
{
    //echo "delete_ontology..." ;
    FnDelOnt($_POST['drupal_id'], $_POST['prolog_id']);
}

else if ($action == "delete_draft")
{
     //echo "delete_draft...";
     FnDelDr($_POST['drupal_id']);

}

else if ($action == "main_ver")
{
    //echo "main version created...."
    FnMakeMainVer($_POST['curcnpt_id'], $_POST['version']);
}

else if ($action == "Change_main_ver")
{
    //echo "main version changed...."
    FnChMainVer($_POST['curcnpt_id'], $_POST['main_id']);
}

else if ($action == "delete_group")
{
    //echo "main version changed...."
    FnDelGroup($_POST['group_id']);
}

?>

<?php

// Функция создает новую node
// Параметры: тип node, статус, на главной странице, приклеена, название, тело, тизер
function make_new_node($type, $status, $promote, $sticky, $title, $body, $teaser)
{
	global $user;
	$node = new StdClass();

	$node->type = $type;

	// Создается от имени текущего пользователя
	$node->uid = $user->uid;

	// Опубликовано
	$node->status = $status;

	// На главную страницу
	$node->promote = $promote;

	$node->sticky = $sticky;
	$node->title = $title;

	// Для онтологии необходимо оставить текст пустым, т.к. текст берется views из др. таблицы
	if ($type == "content_")
	{
		$node->body = "";
	}

	else
	{
		$node->body = $body;
	}


	$node->teaser = $teaser;
	$node->comment = '2';

	node_save($node);
	$nid = $node->nid;

	return $nid;
}

?>

<html>

<head>
  <title>Сообщение системы</title>
  <style type="text/css">
    html {
	background-color: #edf1f3;     
    }
    body 
    {
	margin-bottom: 10px;
	margin-left: auto;
	margin-right: auto;
	margin-top: 25px;
	max-width: 650px;
	overflow-x: hidden;
	padding-bottom: 40px;
	padding-left: 40px;
	padding-right: 40px;
	padding-top: 40px;
	text-align: left;
	overflow-x: visible;
	overflow-y: visible;
	background-color: white;
	border: 1px solid black;
	-webkit-border-radius: 12px;
	-khtml-border-radius: 12px;	
	-moz-border-radius: 12px;
	border-radius: 12px;	
    }
    h1 { 
	color: #ae4444;
	font-size: 1.8em;
	line-height: 0.9;
	margin-bottom: 0.2em;
	margin-left: 0px;
	margin-right: 0px;
	margin-top: 0px;
	text-shadow: 0px 1px;
    }
    a {
	display:block;
	color: #276db4;
    }
  </style>
</head>

<body

<?php
switch ($action)
{
case "delete_draft":
case "delete_ontology":
   break;
default:
   echo "onload = window.close();>";
   break;
};
?>
  </body>
</html>