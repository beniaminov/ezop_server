<script language=Javascript>

function MyFn(nodeid)
{	
	// Подготавливаем ID понятия
	RestoreForm2.draft_id.value = nodeid;
	alert(RestoreForm2.draft_id.value);

	// Теперь подготавливаем текст
	RestoreForm2.draft_id.value = nodeid;
	alert(RestoreForm2.draft_id.value);

	RestoreForm2.submit();
}

</script>

<div>

<!-- пока в целях отладки передается "Сделать средой ядро" -->
<form action="exe/editor.exe"  method="POST" target = "_blank" name="RestoreForm">
	 <input type = "hidden" id = "menu_item" name = "menu_item" value = "cnpt_intoenv"> 
	 <input type = "hidden" id = "env_id" name = "env_id" value = "init.ntn">
	 <input type = "hidden" id = "inset" name = "inset" value = "MakeEnvironment">	 	 	
	 <input type="button" style="visibility: hidden;" id="btn" name = "btn" value = "Сделать средой" onclick = "submit();">
</form>

<form action="exe/editor.exe"  method="POST" target = "_blank" name="RestoreForm2">
	 <input type = "hidden" id = "menu_item" name = "menu_item" value = "restore_concept"> 
	 <input type = "hidden" id = "env_id" name = "env_id" value = "init.ntn">
	 <input type = "hidden" id = "inset" name = "inset" value = "MakeEnvironment">	
	 <input type = "hidden" id = "draft_id" name = "draft_id" value = "">	
	 <input type = "hidden" id = "draft_text" name = "draft_text" value = "">	
	 <input type="button" style="visibility: hidden;" id="btn" name = "btn" value = "Сделать средой" onclick = "submit();">
</form> 

<h2>Личные данные</h2>

<br><b>Имя пользователя:</b> <?php print $user->name ?>
<br><b>Город:</b> <?php print $user->profile_city ?>
<br><b>Страна:</b> <?php print $user->profile_country ?>

<?php
echo "<h2>Сведения о работе в системе</h2>"; ?>

<br><b>Группы:</b> 
<?php 
	$groups_q = db_query("SELECT node.title as title, node.nid as id FROM og_uid, node WHERE og_uid.is_active=1 AND og_uid.nid = node.nid  AND og_uid.uid = " .$user->uid);
	$groups = "";
	while ($node = db_fetch_object($groups_q)) 
		$groups .= "<a href='?q=node/".$node->id."'>".$node->title."</a>, ";
	print  trim($groups,", ");
?>
<br><b>Роли:</b> 
<?php 
	$roles = "";
	foreach($user->roles as $k => $v) 
		if ($k > 2) 
			$roles .=  $v.", "; 
	print trim($roles,", ");
?>


<?php $nlimit = 100; 
      $skip = 0;
	if (isset($_REQUEST['skip']))
	{
	   if(!empty($_REQUEST['skip']))
	   {
		 $skip = $_REQUEST['skip'];
	   }
	}
?>
<?php $userid=$user->uid; ?>
 
<?php $result1 = pager_query(db_rewrite_sql("SELECT n.nid, n.created, n.title FROM node n WHERE n.uid = $userid AND n.type = 'content_' ORDER BY n.created DESC"), $nlimit); 
$format = 'd.m.Y G:i:s';
?>


<?php 
	// Работа с онтологиями
	if (mysql_num_rows($result1) > 20) 
		$output2 .= "<ul style='height: 310px;overflow-y: auto;width:500px;border:1px solid #3399cc;'>";
	else
		$output2 .= "<ul>";
	
	while ($node = db_fetch_object($result1)) 
	{
		$output_date_cont = date($format, $node->created);
		$output2 .= "<li><a href=".$node->node_url."?q=node/".$node->nid.">".$node->title."</a>,"." создано: ".$output_date_cont."</li>";
	}; 
	$output2 .= "</ul>";
	$output2 .= theme('pager', NULL, $nlimit, 2);
?>


<?php $result2 = pager_query(db_rewrite_sql("SELECT n.nid, n.created, n.title
FROM node n WHERE n.uid = $userid AND n.type = 'draft' ORDER BY n.created DESC"), variable_get('default_nodes_main', $nlimit));

?>


<?php 
	// Работа с черновиками
	
	global $result;
	global $cur_nid;
	
	if (mysql_num_rows($result2) > 20) 
		$output3 .= "<ul style='height: 310px;overflow-y: auto;width:500px;border:1px solid #3399cc;'>";
	else
		$output3 .= "<ul>";
	
	while ($node_draft = db_fetch_object($result2)) 
	{	
		$output_date = date($format, $node_draft->created);
		
		echo "<script language=Javascript>RestoreForm2.draft_id.value = $node_draft->nid;</script>";
		$output3 .= "<li><a href=".$node_draft->node_url."?q=node/".$node_draft->nid.">".$node_draft->title."</a>"." создано: ".$output_date."</li>";
		
	}; 

	$output3 .= "</ul>";
	$output3 .= theme('pager', NULL, $nlimit, 3);


?>

<!-- Ссылки на онтологии текущего пользователя, добавить проверку на пустоту -->
<h6>Мои онтологии:</h6>
<?php print $output2; ?> 


<!-- Ссылки на черновики текущего пользователя, добавить проверку на пустоту -->
<h6>Мои черновики:</h6>
<?php print $output3; ?> 

<?php

function MakeCurrent($nid)
{
	$result = $nid;
	
	// Получаем название черновика и его текст
	$title_tmp = db_query("SELECT title FROM node where nid = %s", $nid);
	$title = db_query($title_tmp);
}

?>




</div>

