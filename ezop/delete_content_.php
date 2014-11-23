<?php
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$aquery = db_query("SELECT n.nid FROM {node} n WHERE n.type = 'content_'");
if(db_num_rows($aquery) && user_access('administer nodes')) 
{
    while ($n = db_fetch_object($aquery)) 
    {
       set_time_limit(5);
       node_delete($n->nid);
    }
    db_query("DELETE FROM content_type_id_match");
    db_query("DELETE FROM node_revisions");
    db_query("DELETE FROM content_type_environment"); 
    db_query("DELETE FROM content_type_using");
    db_query("DELETE FROM content_type_version"); 
    db_query("DELETE FROM сontent_type_content_");
    echo 'You can proceed directly to the ' . l('import node wizard','admin/node/node_import') . ' to import new data.';
}
else 
{
  echo "No content_ found or you do not have permission to modify nodes.";
}
?>