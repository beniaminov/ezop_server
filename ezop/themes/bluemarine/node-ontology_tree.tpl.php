<?
//echo "Здесь будет дерево онтологий";

$dblocation = "localhost";
//$dbname = "tree_make";
$dbname = "drupal5_new";
$dbuser = "nobody";
$dbpasswd = "123456";


$dbcnx = @mysql_connect($dblocation, $dbuser, $dbpasswd);

if (!$dbcnx)
{
	echo "Cannot connect to mysql server";
}

if (!@mysql_select_db($dbname, $dbcnx))
{
	echo "Cannot select DB";
	exit();
}

echo "<br><i>Versions Tree<br></i>";
function get_tree($parent_id, $prefix = "") 
{
      global $out;
      //$query = "SELECT * FROM catalogs WHERE parent_id = '$parent_id'";
      
	  $query = "SELECT DISTINCT t1.field_child_id_value, t1.field_parent_id_value, t2.title
				FROM node t2, content_type_content_ t3, content_type_using t1
				WHERE t2.type = 'content_'
				AND t2.nid = t3.nid
				AND t1.field_child_id_value = t3.concept_id 
				AND t1. field_parent_id_value = '$parent_id'
				order by t1.field_parent_id_value";
	  
	  $result = mysql_query($query);

      while ($row = mysql_fetch_array($result)) 
	  {
			//$out .= '<A HREF=ssjhdf'.$row["t2.title"].'</A>';
			
			$out .= $prefix.$row['title']."<br>";
            get_tree($row['field_child_id_value'], $prefix."&nbsp;&nbsp;");
			//get_tree($row['field_parent_id_value'], $prefix."&nbsp;&nbsp;");
      }
      return $out;
}
echo get_tree(50770239); 

echo "<br>New tree: <br>";
function ShowTree($ParentID, $lvl) 
{ 

	global $link; 
	global $lvl; 
	$lvl++; 

	//$sSQL = "SELECT cat_id, parent_id, cat_name  FROM catalogs WHERE parent_id = '$ParentID' 
	//ORDER BY cat_name";
	
	$sSQL = "SELECT DISTINCT t1.field_child_id_value, t1.field_parent_id_value, t2.title
			FROM node t2, content_type_content_ t3, content_type_using t1
			WHERE t2.type = 'content_'
			AND t2.nid = t3.nid
			AND t1.field_child_id_value = t3.concept_id";

	$result = mysql_query($sSQL);

	if (mysql_num_rows($result) > 0) 
	{
		echo("<UL>\n");


		while ( $row = mysql_fetch_array($result) ) 
		{
			$ID1 = $row["t1.field_child_id_value"];
			echo("<LI>\n");
			echo("<A HREF=\""."?ID=".$ID1."\">".$row["t2.title"]."</A>"."  \n");
			ShowTree($ID1, $lvl); 
			$lvl--;
		}

	echo("</UL>\n");
	}

}

ShowTree(50770239, 0);

print "!!!";


if (!@mysql_close($dbcnx))
{
	echo "Cannot close connection";
} 


?>