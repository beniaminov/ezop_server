<?php
// $Id: taxonomy_access_admin.inc,v 1.9.2.1 2007/02/03 15:42:06 keve Exp $

/**
 * @file
 * Administrative interface for taxonomy access control.
 */

/**
 * Menu callback; presents the adminitrator settings page for TAC (/admin/settings/taxonomy_access).
 */
function _taxonomy_access_admin_settings() {
  //preserving for adding more options soon.
  return system_settings_form($form);
}

/**
 * Save the administrator settings.
 */
function _taxonomy_access_admin_settings_submit($form_id, $form_values) {
  //preserving for adding more options soon.
  $op = $form_values['op'];
  if ($op == t('Reset to defaults')) {
    variable_del('taxonomy_access_enabled');
  }
  if ($op == t('Reset to defaults')) {
    drupal_set_message(t('The configuration options of taxonomy access have been reset to their default values.'));
    drupal_goto('admin/settings/taxonomy_access');
  }
  else {
    drupal_set_message(t('The configuration options of taxonomy access have been saved.'));
  }
  return;
}

/**
 * Menu callback; presents the category permissions page of TAC (admin/user/taxonomy_access).
 */
function _taxonomy_access_permissions_page($edit = array()) {
  global $tac_user_roles;
  $rid = arg(3);
  $output = '';
  $tac_user_roles = user_roles();

  // Making permission matrix only if user_role is choosen
  if (isset($tac_user_roles[$rid])) {
    return drupal_get_form('_taxonomy_access_permissions_form', $rid);
  }
  else {
    return theme_taxonomy_access_permissions_page();
  }
}

/**
 * Renders the main page of category permissions
 */
function theme_taxonomy_access_permissions_page() {
  global $tac_user_roles;

  // Render role/permission overview:
  $header = array(t('Role'), '&nbsp;' );
  $rows = array();
  foreach ($tac_user_roles as $rid => $name) {
   $rows[] = array($name, array('data'=>l(t("edit"),"admin/user/taxonomy_access/$rid"), 'align'=>'right'));
  }

  return theme('table', $header, $rows);
}

/**
 * Generating the category permissions form for choosen user role.
 */

function _taxonomy_access_permissions_form($rid) {
    $node_grant_types = array('view', 'update', 'delete');

    // Get all category permissions
    $perm = taxonomy_access_get_grants($rid);
    $default = taxonomy_access_get_default_grants($rid);
    
    $form['taxonomy_access'] = array( '#tree' => TRUE);

    // Do the row for uncategorized nodes
    foreach ($node_grant_types as $grant) {
      $form['taxonomy_access'][0]['term'][0][$grant] = array(
        '#type' => 'checkbox',
        '#default_value' => $perm[0][$grant],
      );
    }

    $vocabs = taxonomy_get_vocabularies();
    $options1 = array( '3' => '--', '1' => t('Allow all'), '0' => t('Ignore all'), '2' => t('Deny all'));
    $options2 = array( '3' => '--', '1' => t('Select all'), '0' => t('Deselect all'));
    //Radios' values: '1' => t('Allow'), '0' => t('Ignore'), '2' => t('Deny')
    $radios = array('1' => '', '0' => '', '2' => ''); 
    
    foreach ($vocabs as $vocab) {
      $form['taxonomy_access'][$vocab->vid] = array(
        '#title' => $vocab->name,
        '#tree' => TRUE,
       );

      foreach ($node_grant_types as $grant) {
        $form['taxonomy_access'][$vocab->vid]['vocab'][$grant] = array(
          '#type' => 'select', 
          '#options' => $options1,
          '#default_value' => -1,
         );
        $form['taxonomy_access'][$vocab->vid]['default'][$grant] = array(
          '#type' => 'radios', 
          '#options' => $radios,
          '#default_value' => isset($default[$vocab->vid][$grant]) ? $default[$vocab->vid][$grant] : 0,
         );
      }
      foreach (array('create', 'list') as $grant) {
        $form['taxonomy_access'][$vocab->vid]['vocab'][$grant] = array(
          '#type' => 'select', 
          '#options' => $options2,
         );
        $form['taxonomy_access'][$vocab->vid]['default'][$grant] = array(
          '#type' => 'checkbox', 
          '#default_value' => isset($default[$vocab->vid][$grant]) ? $default[$vocab->vid][$grant] : 0,
         );
       }

      // Defining form elements for each term in vocabulary
      $terms = array();
      $terms = taxonomy_get_tree($vocab->vid);
      if ($terms) {
        foreach ($terms as $term) {
          foreach ($node_grant_types as $grant) {
            $form['taxonomy_access'][$vocab->vid]['term'][$term->tid][$grant] = array(
              '#type' => 'radios', 
              '#options' => $radios,
              '#default_value' => isset($perm[$term->tid][$grant]) ? $perm[$term->tid][$grant] : 0,
             );
          }
          foreach (array('create', 'list') as $grant) {
            $form['taxonomy_access'][$vocab->vid]['term'][$term->tid][$grant] = array (
              '#type' => 'checkbox', 
              '#default_value' => isset($perm[$term->tid][$grant]) ? $perm[$term->tid][$grant] : 0,
             );
          }
        }
      }
    }

    $form['taxonomy_access']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save category permissions'),
      '#name' => 'save',
      '#button_type' => 'submit',
    );
  return $form;
}

/**
 * Renders the permission matrix user form for choosen user role.
 */
function theme__taxonomy_access_permissions_form($form) {
  global $tac_user_roles;
  $rid = arg(3);

  $vids = array_keys($form['taxonomy_access']);
  $node_grant_types = array('view', 'update', 'delete');

  $header = array();
  $header[] = array( 'data'=>t('Category')); 
  $header[] = array( 'data'=>t('View'), 'colspan'=>4);
  $header[] = array( 'data'=>t('Update'), 'colspan'=>4);
  $header[] = array( 'data'=>t('Delete'), 'colspan'=>4);
  $header[] = array( 'data'=>t('Create'));
  $header[] = array( 'data'=>t('List'));

  $sub_header[] = '&nbsp;<strong>'.t('A').'</strong>';
  $sub_header[] = '&nbsp;<strong>'.t('I').'</strong>';
  $sub_header[] = '&nbsp;<strong>'.t('D').'</strong>';
  $sub_header[] = '&nbsp;';

  $radios = array('1' => t('Allow'), '0' => t('Ignore'), '2' => t('Deny'));

  $output = '<h2>'.t('Permissions for')." '".$tac_user_roles[$rid]."'</h2><p>";
  foreach ($vids as $vid) {
    if (is_integer($vid)) {
      $rows = array();
      if ($vid == 0) {
        // Do the row for uncategorized nodes
        $row = array();
        $row[] = '<strong>'.t('Uncategorized nodes').'</strong>';
        foreach ($node_grant_types as $grant) {
          $row[] = array('data' => drupal_render($form['taxonomy_access'][0]['term'][0][$grant]), 'colspan'=>4);
        }
        $row[] = '&nbsp;';
        $row[] = '&nbsp;';
        $rows[] = $row; 
        $output_table = theme('table', $header, $rows);
        $output .= theme_fieldset(array('#title'=>'Uncategorized nodes', '#value'=>$output_table, '#collapsible' => TRUE, '#collapsed' => TRUE));
      }
      else {
        //1st row: 'Allow all'... 'Select All'...
        unset($row);
        $row = array();
        $row[] = array('data' => '<strong>'.$form['taxonomy_access'][$vid]['#title'].'</strong>');
        foreach ($node_grant_types as $grant) {
          $row[] = array('data' => drupal_render($form['taxonomy_access'][$vid]['vocab'][$grant]), 'colspan'=>4);
        }
        foreach (array('create', 'list') as $grant) {
          $row[] = array('data' => drupal_render($form['taxonomy_access'][$vid]['vocab'][$grant]));
        }
        $rows[] = $row;

        //2nd row: 'A' 'I' 'D'
        $row = array();
        $row[] = '&nbsp;';
        foreach ($node_grant_types as $grant) {
          $row = array_merge($row, $sub_header);
        }
        $row[] = array('data'=>'&nbsp;', 'colspan'=>2);
        $rows[] = $row;

        //3rd row: Defaults
        $row = array();
        $row[] = '<blockquote><em>'.t('Default').'</em></blockquote>';
        foreach (array('view', 'update', 'delete') as $grant) {
          foreach (array_keys($radios) as $key) {
            // I need this hack to display radio buttons horizontally (instead of standard form 'radios')
            $row[] = array('data' => drupal_render($form['taxonomy_access'][$vid]['default'][$grant][$key]));
          }
          $row[] = '&nbsp;&nbsp;';
        }
        foreach (array('create', 'list') as $grant) {
          $row[] = array('data' => drupal_render($form['taxonomy_access'][$vid]['default'][$grant]), 'align' => 'center');
        }
        $rows[] = $row;

        //4th row: Empty row
        $row = array();
        $row[] = array('data'=>'&nbsp;', 'colspan'=>15);
        $rows[] = $row;

        // Rows for each term in vocabulary
        $terms = array();
        $terms = taxonomy_get_tree($vid);

        if ($terms) {
          foreach ($terms as $term) {
            $row = array();
            $row[] = str_repeat('-',$term->depth).$term->name;
            foreach ($node_grant_types as $grant) {
            // I need this hack to display radio buttons horizontally (instead of standard form 'radios')
              foreach (array_keys($radios) as $key) {
                $row[] = drupal_render($form['taxonomy_access'][$vid]['term'][$term->tid][$grant][$key]);
              }
              $row[] = '&nbsp;&nbsp;';
            }
            foreach (array('create', 'list') as $grant) {
              $row[] = array('data' => drupal_render($form['taxonomy_access'][$vid]['term'][$term->tid][$grant]), 'align' => 'center');
            }
          $rows[] = $row;
          }
        }

        $output_table = theme('table', $header, $rows);
        $output .= theme_fieldset(array('#title'=>$form['taxonomy_access'][$vid]['#title'], '#value'=>$output_table, '#collapsible' => TRUE, '#collapsed' => TRUE));

      }
    }
  }
  $output .= drupal_render($form);

  return $output;
}

/**
 * Saves the category permissions matrix for choosen user role, after editing.
 */
function _taxonomy_access_permissions_form_submit($form_id, $form_values) {
  global $tac_user_roles;
  $vids = array_keys($form_values['taxonomy_access']);
  $rid = arg(3);
  $grant_types = array('view', 'update', 'delete', 'create', 'list');
  $edit = $form_values['taxonomy_access'];

  foreach ($vids as $vid) {
    if (is_numeric($vid)) {
    // Process input data and use it to make changes to the database.
      if (is_array($edit[$vid]['term'])) {
        $tids = array_keys($edit[$vid]['term']);
        foreach ($tids as $tid) {
          // The $edit array is filled out with zeroes here to
          // reduce code complexity when we update the term_access table.
          if (!$edit[$vid]['term'][$tid]) {
            foreach ($grant_types as $grant) {
              $edit[$vid]['term'][$tid][$grant] = 0;
            }
          }
          else {
              foreach ($grant_types as $grant) {
                if (!isset($edit[$vid][$tid][$grant])) {
                  // Enter a '0' for any permission that has not been set or for any checkbox that has been left unchecked by the user
                  $edit[$vid][$tid][$grant] = 0;
                }
                else {
                  if ($grant == 'create' || $grant == 'list') {
                    // For any permission checkbox checked by user, set it to a value of '1'
                    $edit[$vid][$tid][$grant] = 1;
                  }
                }
              }
          }
          
          // Make the changes to the term_access table
          taxonomy_access_grant_update($tid, $rid, $edit[$vid]['term'][$tid]);
        }
      }

    // Now to the same for vocabularies
      if ($vid !=0) {
        // The $edit array is filled out with zeroes here to
        // reduce code complexity when we update the term_access table.
        $changed = false;
        if (!array_key_exists($vid, $edit)) {
          foreach ($grant_types as $grant) {
            $edit[$vid]['vocab'][$grant] = 0;
          }
        }
        else {
          foreach ($grant_types as $grant) {
            if (($edit[$vid]['vocab'][$grant] === NULL) || !in_array($edit[$vid]['vocab'][$grant], array( 0, 1, 2))) {
              $edit[$vid]['vocab'][$grant] = 3;
            }
            else {
              $changed = true;
            }
            // Enter a '0' for any permission that has been left unchecked by the user
            // For any permission checked by user, set it to a value of '1'
            if ($grant == 'create' || $grant == 'list') {
              $edit[$vid]['default'][$grant] = $edit[$vid]['default'][$grant] ? 1 : 0;
            }
            else {
              $edit[$vid]['default'][$grant] = $edit[$vid]['default'][$grant][0];
            }
          }
        }
        // Make the changes to the term_access table
        if ($changed) {
          taxonomy_access_grant_vocab_update($vid, $rid, $edit);
        }
        // set the defaults
        taxonomy_access_defaults_update($vid,$rid,$edit[$vid]['default']);
      }
    }
  }

  // Force to rebuild the node_access table according changes made to term_access table

  //Todo: 1. Possibly optimizing when values changes for only one user role (permission matrix submit)
  //         _refresh_node_access_table($rid);
  //      2. Run this when changes were made to 'View/Update/Delete' permissions
  node_access_rebuild();

  // redirect to main category permissions menu
  //  $roles = user_roles();
  drupal_set_message(t("Your permission settings for '%role' role have been saved.", array('%role' => $tac_user_roles[$rid])), 'status');
  drupal_goto('admin/user/taxonomy_access');

  return;
}
