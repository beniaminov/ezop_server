<?php

//  Не помещать НИКАКОЙ вывод ДО этих строк, иначе будет ошибка заголовков!
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

//echo "<br>Inside actions profile";

//echo "<br>".$_GET['act'];
//echo "<br>".$_GET['id'];

$action = $_GET['act'];
$id = $_GET['id'];

if ($action == "d")
{
	// Удаление черновика в БД
	db_query("DELETE FROM node where nid = %s", $id);
}

// Перенаправление обратно на страницу профиля
// №№№№ Ошибка! Адрес должен быть относительным!!!
$location = "Location: http://localhost/drupal/?q=user/".$_GET['u'];

//echo "<br>".$location;
//echo "<br>".$_SERVER['REMOTE_HOST'];

header ($location); 

?>