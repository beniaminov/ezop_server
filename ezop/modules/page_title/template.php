<?php

function _phptemplate_variables($hook, $vars) {
  $vars = array();
  if ($hook == 'page') {
    if (module_exists('page_title')) {
      $vars['head_title'] = page_title_page_get_title();
    }
  }
  return $vars;
}
