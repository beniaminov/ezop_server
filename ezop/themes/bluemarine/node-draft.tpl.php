  <div class="node<?php if ($sticky) { print " sticky"; } ?><?php if (!$status) { print " node-unpublished"; } ?>">
    <?php if ($picture) {
      print $picture;
    }?>
    <?php if ($page == 0) { ?><h2 class="title"><a href="<?php print $node_url?>"><?php print $title?></a></h2><?php }; ?>
    <span class="submitted"><?php print $submitted?></span>
    <span class="taxonomy"><?php print $terms?></span>
    <div class="content"><?php print $content?></div>
    <?php if ($links) { ?><div class="links">&raquo; <?php print $links?></div><?php }; ?>

	<?php
	//ID онтологии(черновика) в прологе
	$res_q = db_query("SELECT field_nid_prolog FROM draft_info WHERE nid = %s", $nid);
	$result = db_result($res_q);
	//echo "<br>ID в БД пролог: ".$result;
	//echo "<br>ID в БД друпал: ".$nid;

	//тело онтологии(черновика) в друпал
	$res_q_body = db_query("SELECT body FROM node_revisions WHERE nid = %s", $nid);
	$result_body = db_result($res_q_body);
	//echo "<br>тело в БД Drupal: ".$result_body;

	//название онтологии(черновика) в друпал
	$res_q_title = db_query("SELECT title FROM node WHERE nid = %s", $nid);
	$result_title = db_result($res_q_title);
	//echo "<br>имя в БД Drupal: ".$result_title;

	//среда онтологии(черновика) в друпал
	$res_q_env = db_query("SELECT field_id__drupal_value FROM draft_info WHERE nid = %s", $nid);
	$result_env = db_result($res_q_env);
	//echo "<br>среда в БД Drupal: ".$result_env;

	//используемы id  онтологии(черновика) в друпал
	$res_q_used = db_query("SELECT field_parent_id_value  FROM draft_info WHERE nid = %s", $nid);
	$result_used = db_result($res_q_used);
	$result_used_string = (string)$result_used;
	//echo "<br>использует в БД Drupal: ".$result_used_string;

	//Определение пользователя
	global $user;
	//echo "<br>пользователь: ".$user->name;


	?>

	<TABLE WIDTH="70">
		<TR>
			<TD>
				<?php 
					$action = "exe/editor.exe";
					if (arg(0) == 'node' && is_numeric(arg(1)))
					{
						$nodeid = arg(1);
						$group_nid_q = db_query("SELECT group_nid FROM og_ancestry WHERE nid = ".$nodeid);
						if(db_num_rows($group_nid_q) > 0)
						{
							$group_nid = db_result($group_nid_q);
							$action .= "?group=".$group_nid;
						}
					} 
				?>
	 			<form action="<?php print $action ?>"  method="POST" target = "_blank">
					<input type="hidden" id="menu_item" name = "menu_item" value = "CC_editdraft">
					<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "<?php print $result ?>">
					<input type="hidden" id="old_id" name = "old_id" value = "">
					<input type="hidden" id="curcnpt_name" name = "curcnpt_name" value = "<?php print $result_title ?>">
					<input type="hidden" id="env_id" name = "env_id" value = "<?php print $result_env ?>">
					<input type="hidden" id="curcnpt_text" name = "curcnpt_text" value = "<?php print htmlentities($result_body, ENT_QUOTES, 'UTF-8') ?>">
					<input type="hidden" id="rest" name = "rest" value = "">
					<input type="hidden" id="usedIds" name = "usedIds" value = "<?php print $result_used ?>">
					<input type="hidden" id="inset" name = "inset" value = "CC">
					<input type="hidden" id="user" name = "user" value = "<?php print $user->name ?>">
					<input type="button" id="button" value = "Доработать черновик" onclick = "submit();">
				</form>
			</TD>
			<?php 
				$returnURL = "?q=user/". $user->uid;
				$group_nid = db_result(db_query("SELECT group_nid FROM og_ancestry WHERE nid = ".$nodeid));
				if($group_nid > 0)
					$returnURL = "?q=node/" . $group_nid;
			?>
			<TD>
	 			<form action="exe/editor.exe"  method="POST">
					<input type="hidden" id="menu_item" name = "menu_item" value = "delete_ontology">
					<input type="hidden" id="curcnpt_id" name = "curcnpt_id" value = "<?php print $result ?>">
					<input type="hidden" id="inset" name = "inset" value = "delete_draft">
					<input type="hidden" id="old_id" name = "old_id" value = "">
		                        <input type="hidden" id="drupal_id" name = "drupal_id" value = "<?php print $nid ?>">
		                        <input type="hidden" id="curcnpt_name" name = "curcnpt_name" value = "<?php print $result_title ?>">
					<input type="button" id="button" value = "Удалить черновик" onclick="if (confirm('Удалить черновик?')) { submit(); }">
				</form>
			</TD>
		</TR>
	</TABLE>
  </div>