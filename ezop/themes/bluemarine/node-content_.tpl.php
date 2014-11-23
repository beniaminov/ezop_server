  <div class="node<?php if ($sticky) { print " sticky"; } ?><?php if (!$status) { print " node-unpublished"; } ?>">
    <?php if ($picture) {
      print $picture;
    }?>
    <?php if ($page == 0) { ?><h2 class="title"><a href="<?php print $node_url?>"><?php print $title?></a></h2><?php }; ?>
    <span class="submitted"><?php print $submitted?></span>
    <span class="taxonomy"><?php print $terms?></span>
    <div class="content"><?php print $content?></div> 
    <?php if ($links) { ?><div class="links">&raquo; <?php print $links?></div><?php }; ?>

	<!-- По ID онтологии в Друпал получаем ID онтологии Пролог для последующей передачи -->
	<?php 
	$res_q = db_query("SELECT field_nid_prolog FROM content_type_id_match WHERE field_id_drupal_value=%s", $nid); 
	$result = db_result($res_q);

	//Заполняем показатель главности - checkbox
	$res_check = db_query("SELECT main_version FROM content_version WHERE nid=%s", $nid); 
	$result_check = db_result($res_check);
	//echo "<br>версия: ".$result_check;
	//echo( ' <input type="checkbox"  name="checkbox" value="1" disabled ="disabled"  /> Выбрано главной версией  <br><br>'  );
	

	if($result_check==1)
	{
		echo( ' <input type="checkbox"  name="checkbox" value="1" disabled ="disabled"  checked="checked" />Выбрано главной версией <br><br> '  );
	}
	else
	{
		Ch($result);
	}

	//Ищем в таблице версий, все версии, связанные с этой онтологией
function Ch($result)
{
	//echo $result;
	$res_ver_ch = db_query("SELECT field_child_id_0_value FROM content_type_version WHERE field_parent_id_0_value=%s", $result); 
	$res_ver_par = db_query("SELECT field_parent_id_0_value FROM content_type_version WHERE field_child_id_0_value=%s", $result); 
	$result_parent = db_result($res_ver_par);
	
	if(db_num_rows($res_ver_par)>0)
	{
		//echo "есть предки!";
		$x = 0;
		//echo "работает1";
		$res_version1 = db_query("SELECT nid FROM content_version WHERE main_version = 1 AND nid_prolog = %s",$result_parent); 
		//echo "работает2";
		$result_version1 = db_result($res_version1);
		//echo "работает3";
		if($result_version1>0)
		{
			$x++;
		}

		//echo "x после=".$x;
		if($x==0)
		{
		//	echo "работаем с детьми1";
			$res_ver_ch_all = db_query("SELECT field_child_id_0_value FROM content_type_version WHERE field_parent_id_0_value=%s", $result_parent); 
		//	echo "работаем с детьми2";
			$y = 0;
		//	echo "работает с детьми3";
			while ($child = db_fetch_array($res_ver_ch_all)) 
			{
		//		echo "работает11";
				$res_version3 = db_query("SELECT nid FROM content_version WHERE main_version = 1 AND nid_prolog=%s", $child); 
		//		echo "работает21";
				$result_version3 = db_result($res_version3);
		//		echo "работает3";
				if($result_version3>0)
				{
					$y++;
				}
			}
		//	echo "а вот теперь у = ".$y;
			if($y>0)
			{
				echo "Главная версия уже выбрана!";
				echo( '<form action="exe/editor.exe"  method="POST" target = "_blank">');
				echo( '<input type="hidden" id="menu_item" name = "menu_item" value = "ChangeMain_ver">');
				echo( '<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "'. $result .'">');
				echo( '<input type="hidden" id="main_id" name = "main_id" value = "'. $result_version3 .'">');
				echo( '<input type="hidden" id="version" name = "version" value = "1">');
				echo( '<input type="button" id="button" value = "Изменить главную версию на текущую" onclick = "submit();">');
				echo( '</form>'  );
			}
			else
			{
				//echo "ура!";
				echo( ' <form action="exe/editor.exe"  method="POST" target = "_blank"><input type="hidden" id="menu_item" name = "menu_item" value = "Main_ver"> <input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "'. $result .'"><input type="hidden" id="version" name = "version" value = "1"> <input type="button" id="button" value = "Сделать главной версией" onclick = "submit();"></form>'  );
			}
		}
		else
		{
			echo "Главная версия уже выбрана!!!!";
				echo( '<form action="exe/editor.exe"  method="POST" target = "_blank">');
				echo( '<input type="hidden" id="menu_item" name = "menu_item" value = "ChangeMain_ver">');
				echo( '<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "'. $result .'">');
				echo( '<input type="hidden" id="main_id" name = "main_id"  value = "'. $result_version1 .'">');
				echo( '<input type="hidden" id="version" name = "version" value = "1">');
				echo( '<input type="button" id="button" value = "Изменить главную версию на текущую" onclick = "submit();">');
				echo( '</form>'  );
		}

	}
	else 
	{
		if(db_num_rows($res_ver_ch)>0)
		{
			echo "есть дети!";
			$x = 0;
			while ($node = db_fetch_array($res_ver_ch)) 
			{
				echo "работает1";
				$res_version2 = db_query("SELECT nid FROM content_version WHERE main_version = 1 AND nid_prolog = %s", $node); 
				echo "работает2";
				$result_version2 = db_result($res_version2);
				echo "работает3";
				if($result_version2>0)
				{
					$x++;
				}
			}
			echo "x после=".$x;
			if($x==0)
			{
				echo "ура!";
				echo( ' <form action="exe/editor.exe"  method="POST" target = "_blank"><input type="hidden" id="menu_item" name = "menu_item" value = "Main_ver"> <input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "'. $result .'"><input type="hidden" id="version" name = "version" value = "1"> <input type="button" id="button" value = "Сделать главной версией" onclick = "submit();"></form>'  );
			}
			else
			{
				echo "Главная версия уже выбрана!";
				echo( '<form action="exe/editor.exe"  method="POST" target = "_blank">');
				echo( '<input type="hidden" id="menu_item" name = "menu_item" value = "ChangeMain_ver">');
				echo( '<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "'. $result .'">');
				echo( '<input type="hidden" id="main_id" name = "main_id" value = "'. $result_version2 .'">');
				echo( '<input type="hidden" id="version" name = "version" value = "1">');
				echo( '<input type="button" id="button" value = "Изменить главную версию на текущую" onclick = "submit();">');
				echo( '</form>'  );
			}
		}
		else
		{
			echo "у данной онтологии нет версий";
		}
	}
}
	//Определение пользователя
	global $user;
	//echo "<br>пользователь: ".$user->name;

	?>	
	<TABLE WIDTH="70">
		<TR><TD></TD></TR><TR><TD></TD></TR><TR><TD></TD></TR><TR><TD></TD></TR>

		<TR>
			<TD>
				<?php 
					$action = "exe/editor.exe";
					$delBtnVisible = false;
					$makeEnvBtnVisible = false;
					if (arg(0) == 'node' && is_numeric(arg(1)))
					{
						//Определяем, в группе ли мы находимся
						$nodeid = arg(1);
						$group_nid_q = db_query("SELECT group_nid FROM og_ancestry WHERE nid = ".$nodeid);
						if(db_num_rows($group_nid_q) > 0)
						{
							$group_nid = db_result($group_nid_q);
							$action .= "?group=".$group_nid;
						}
						
						//Определяем, может ли пользователь удалять онтологию
						if($user->uid > 0)
						{
							$user_is_author = db_result(db_query("SELECT count(*) FROM node WHERE nid = ".$nodeid." AND uid = ".$user->uid));
							$user_is_group_admin = db_result(db_query("SELECT count(*) FROM og_ancestry where nid = ".$nodeid." AND group_nid in (select og_uid.nid from og_uid where og_uid.is_active = 1 AND og_uid.is_admin = 1 AND og_uid.uid = " . $user->uid . ")"));
							$user_is_global_admin = (in_array('модератор', array_values($user->roles)) || $user->uid == 1) ? 1 : 0;
							//echo $user_is_author;echo $user_is_group_admin;echo $user_is_global_admin;
							if($user_is_author + $user_is_group_admin + $user_is_global_admin > 0)
								$delBtnVisible = true;
							//$makeEnvBtnVisible = true;
						}
					} 
				?>
				<?php  if($makeEnvBtnVisible) { ?>
	 			<form action="<?php print $action ?>"  method="POST" target = "_blank">
					<input type = "hidden" id = "menu_item" name = "menu_item" value = "cnpt_intoenv"> 
	 				<input type = "hidden" id = "env_id" name = "env_id" value = "<?php print $result ?>">
	 				<input type = "hidden" id = "inset" name = "inset" value = "MakeEnvironment">
					<input type="hidden" id="user" name = "user" value = "<?php print $user->name ?>">					
	 				<input type="button" id="button" value = "Сделать средой" onclick = "submit();">
				 </form> 
				<?php } ?>
			</TD>	

			
			<TD>
	 			<form action="exe/editor.exe"  method="POST" target = "_blank">
					<input type="hidden" id="menu_item" name = "menu_item" value = "new_command"> 
					<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "<?php print $result ?>">
					<input type="hidden" id="old_id" name = "old_id" value = ""> 		
					<input type="hidden" id="user" name = "user" value = "<?php print $user->name ?>">
					<input type="button" id="button" value = "Задать вопрос" onclick = "submit();">
				</form>
			</TD>	

			<TD>
				<form style="display:none" action="exe/editor.exe"  method="POST" target = "_blank">
					<input type="hidden" id="menu_item" name = "menu_item" value = "CC_newver"> 
					<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "<?php print $result ?>">
					<input type = "hidden" id = "inset" name = "inset" value = "CC_newver">	
					<input type="hidden" id="user" name = "user" value = "<?php print $user->name ?>">
					<input type="button" id="button" value = "Создать новую версию" onclick = "submit();">
				</form>
				
			</TD>

			
			<?php 
				if($delBtnVisible) { ?>
			<TD>
	 			<form action="exe/editor.exe"  method="POST" >
					<input type="hidden" id="menu_item" name = "menu_item" value = "delete_ontology"> 
					<input type="hidden" id="curcnpt_id" name = "prolog_id" value = "<?php print $result ?>">
					<input type="hidden" id="old_id" name = "drupal_id" value = "<?php print $nodeid ?>"> 			
					<input type="button" id="button" onclick="if (confirm('Удалить понятие?')) { submit(); }" value = "Удалить понятие" >
				</form>
			</TD>	
			<?php } ?>
		</TR>
	</TABLE>
	<br/>
	<input type="button" value="Экспорт" onclick="show();" id="BSHOW" style="width:100"><br/><br/>
	<TABLE ID="TEXPORT" style="display:none">	
		<TR>
			<TD>
	 			<form action="./scripts/ex/ex.py"  method="POST" target="_self">
				    <input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "<?php print $result ?>">
				    <input type="hidden" id="curcnpt_title" name = "curcnpt_title" value = "<?php print $title ?>">
				    <fieldset>
				    <legend>Размер онтологии:</legend>
				    <input type = "radio" name = "size" value = "full">Вся онтология, включая зависимости ядра;<br>
				    <input type = "radio" name = "size" value = "small" checked>Только новые понятия, определенные в текущей онтологии.<br>
				    </fieldset>
				    <fieldset>
				    <legend>Тип результата:</legend>
				    <input type = "radio" name = "type" value = "owl">Файл OWL;<br>
				    <input type = "radio" name = "type" value = "pic" checked>Схема онтологии.<br>
				    </fieldset>
				    <input type="button" id="button" value = "Ок" onclick = "submit();" style="width:100">
				</form>
			</TD>
		</TR>
	</TABLE>
  </div>
  <script type="text/javascript">
  function show(){
    table = document.getElementById('TEXPORT');
    if (table.style.display=='none'){
      table.style.display='block';
      document.getElementById('BSHOW').value='Скрыть';
    }
    else {
      table.style.display='none';
      document.getElementById('BSHOW').value='Экспорт';
    }
  }
  </script>