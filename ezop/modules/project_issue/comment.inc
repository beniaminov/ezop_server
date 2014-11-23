<?php
// $Id: comment.inc,v 1.85.2.2 2008/01/30 22:54:57 thehunmonkgroup Exp $
// $Name: DRUPAL-5--1-3 $

function project_comment_page($op, $nid) {

  // Load the parent node.
  $node = node_load($nid);

  if (node_access('create', 'project_issue')) {
    $edit = (object)$_POST;

    if ($_POST['op'] == t('Preview') || $_POST['op'] == t('Submit')) {
      project_comment_validate($edit);
    } else {
      foreach (array('nid', 'type', 'pid', 'rid', 'category', 'component', 'priority', 'assigned', 'sid', 'title') as $var) {
        $edit->$var = $node->$var;
      }
      project_comment_validate($edit);
    }
    $output .= drupal_get_form('project_comment_form', $edit);

    // set breadcrumb
    $project = node_load(array('nid' => $node->pid));
    project_issue_set_breadcrumb($node, $project);

    drupal_set_title(t('New comment'));
    switch ($_POST['op'] ? $_POST['op'] : arg(2)) {
      case 'add':
        $output .= node_view($node, NULL, TRUE);
        return $output;
        break;
      case t('Preview'):
          return $output;
        break;
      case t('Submit'):
          if (!form_get_errors()) {
            $edit->nid = $node->nid;

            project_comment_save($edit);
            drupal_goto("node/$node->nid");
          } else {
            return $output;
          }
        break;
    }
  }
  else {
    drupal_set_message(t('You are not authorized to follow up on issues.'), 'error');
    drupal_goto("node/$nid");
  }
}

function project_comment_form($edit, $param = NULL) {
  $op = $_POST['op'];
  if (isset($param)) {
    $form = array(
      '#method' => $param['method'],
      '#action' => $param['action'],
      '#attributes' => $param['options'],
    );
  } else {
    $form['#attributes'] = array('enctype' => 'multipart/form-data');
  }
  $form['#prefix'] = '<div class="project-issue"><div class="node-form"><div class="standard">';
  $form['#suffix'] = '</div></div></div>';
  $form['project_issue_form'] = project_issue_form($edit, $param);
  unset($form['project_issue_form']['#prefix']);
  unset($form['project_issue_form']['#suffix']);

  // Add any CCK fields to the follow-up form.
  if (function_exists('_content_widget_invoke')) {
    $db_node = node_load(array('nid' => arg(3), 'type' => 'project_issue'));
    $type = content_types($db_node->type);
    _content_widget_invoke('prepare form values', $db_node);
    $form = array_merge($form, _content_widget_invoke('form', $db_node));
  }

  _project_issue_form_add_required_fields($form['project_issue_form'], FALSE);
  if ($edit->cid) {
    $form['cid']= array('#type' => 'hidden', '#value' => $edit->cid);
  }
  $form['preview'] = array('#type' => 'button', '#value' => t('Preview'));
  if (!form_get_errors()) {
    $form['submit'] = array('#type' => 'button', '#value' => t('Submit'));
  }
  if ($op == t('Preview')) {
    $form['#after_build'] = array('project_comment_form_add_preview');
  }
  return $form;
}

function project_comment_validate(&$edit) {
  global $user;

  $edit->uid = $user->uid;
  $edit->name = $user->name;

  if ($edit->cid) {
    $comment = project_comment_load($edit->cid);
    $edit->nid = $comment->nid;
  }

  $edit->comment = true;

  project_issue_comment_validate($edit);
  $edit->validated = true;
}

function project_comment_view($node, $main = 0) {
  global $user;
  $rows = array();
  $result = db_query('SELECT p.*, u.name FROM {project_comments} p INNER JOIN {users} u USING (uid) WHERE p.nid = %d ORDER BY p.created ASC', $node->nid);
  if (db_num_rows($result)) {
    $output = '<div class="project-issue">';
    $i = 0;
    while ($comment = db_fetch_object($result)) {
      $comment->body = db_decode_blob($comment->body);
      $comment->data = db_decode_blob($comment->data);
      $i++;
      $output .= _project_comment_view_single($comment, $i);
    }
    $output .= '</div>';
    return theme('box', t('Updates'), $output);
  }
}

/**
 * Private method to view a single project comment (issue followup).
 *
 * @param $comment
 *   An array or object of the comment to view.
 * @param $count
 *   The integer that shows what number of comment this is.
 *
 * @return
 *   A string of validated output to theme/display.
 *
 */
function _project_comment_view_single($comment, $count) {
  $comment = (object)$comment;
  $summary = array();
  $output = '';

  $fields = array(
    'title' => t('Title'),
    'pid' => t('Project'),
    'rid' => t('Version'),
    'component' => t('Component'),
    'category' => t('Category'),
    'priority' => t('Priority'),
    'assigned' => t('Assigned to'),
    'sid' => t('Status'),
  );

  // If we got this from the DB, we'll have a $data field to unserialize.
  $comment = drupal_unpack($comment);

  // Print out what changed about the issue with this comment. If the
  // comment is in the DB, we'll have 'old' and 'new' fields from the
  // 'data' field, which record exactly what changed. If not, we'll
  // load the origial node and compare against that.
  if (!isset($comment->data)) {
    $node = node_load(array('nid' => arg(3), 'type' => 'project_issue'));
  }
  foreach ($fields as $field => $text) {
    if (isset($comment->old->$field) && isset($comment->new->$field)) {
      $summary[] = array(
        $text .':',
        check_plain(project_mail_summary($field, $comment->old->$field)),
        '&raquo; '. check_plain(project_mail_summary($field, $comment->new->$field))
      );
    }
    elseif (isset($node->$field) && isset($comment->$field) && $node->$field != $comment->$field ) {
      $summary[] = array(
        $text .':',
        check_plain(project_mail_summary($field, $node->$field)),
        '&raquo; '. check_plain(project_mail_summary($field, $comment->$field))
      );
    }
  }

  if ($comment->file_path && file_exists($comment->file_path)) {
    $summary[] = array(t('Attachment:'), '<a href="'. file_create_url($comment->file_path). '">'. basename($comment->file_path) .'</a> ('. format_size($comment->file_size) .')');
  }

  if ($summary || $comment->body) {
    if ($count) {
      $output .= '<div class="header">';
      $output .= t('!count submitted by !user on !date', array('!count' => l("#$count", "node/$comment->nid", array ('id' => "comment-$comment->cid", 'name' => "comment-$comment->cid"), NULL, "comment-$comment->cid"), '!user' => theme('username', $comment), '!date' => format_date($comment->created))) . theme('mark', node_mark($comment->nid, $comment->changed));
      $output .= '</div>';
    }
    if ($summary) {
      $output .= '<div class="summary">';
      $output .= theme('table', array(), $summary);
      $output .= '</div>';
    }
    if ($comment->body) {
      $output .= '<div class="content">';
      $output .= check_markup($comment->body);
      $output .= '</div>';
    }
  }
  return $output;
}

function project_comment_load($cid) {
  $object = db_fetch_object(db_query('SELECT p.*, u.name FROM {project_comments} p INNER JOIN {users} u USING (uid) WHERE p.cid = %d ORDER BY p.created DESC', $cid));
  $object->body = db_decode_blob($object->body);
  $object->data = db_decode_blob($object->data);
  return $object;
}

function project_comment_save($edit) {
  global $user;

  if (empty($edit->cid)) {
    $edit->cid = db_next_id('{project}_cid');
    if ($edit->file) {
      $directory = file_create_path(variable_get('project_directory_issues', 'issues'));
      $edit->file->filename = project_issue_munge_filename($edit->file->filename);
      $file = file_save_upload($edit->file, $directory);
      unset($edit->file);
    }

    if (empty($edit->uid)) {
      $edit->uid = $user->uid;
    }
    $node = node_load(array('nid' => $edit->nid, 'type' => 'project_issue'));

    // Check if comment changed any of the state values and update the node if necessary
    foreach (array('pid', 'rid', 'category', 'component', 'priority', 'assigned', 'sid', 'title') as $var) {
      if ($node->$var != $edit->$var) {
        $data['old']->$var = $node->$var;
        $data['new']->$var = $edit->$var;
        $node->$var = $edit->$var;
      }
    }

    // Add processed cck info into the node object for node_save
    if (function_exists('_content_widget_invoke')) {
      $type = content_types($node->type);
      while (list($field, $val) = each($type['fields'])) {
        $node->$field = $edit->$field;
      }
      _content_widget_invoke('process form values', $node);
    }

    watchdog('content', t('project_issue: added comment %title', array('%title' => $edit->title)), WATCHDOG_NOTICE, l('view', "node/$node->nid"));
    $node->changed = time();
    db_query("INSERT INTO {project_comments} (cid, nid, uid, created, changed, body, data, file_path, file_mime, file_size) VALUES (%d, %d, %d, %d, %d, %b, %b, '%s', '%s', %d)", $edit->cid, $edit->nid, $edit->uid, $node->changed, $node->changed, $edit->body, serialize($data), $file->filepath, $file->filemime, $file->filesize);

    // Update node_comment_statistics so the tracker page lists the number of comments
    $count = db_result(db_query('SELECT COUNT(cid) FROM {project_comments} WHERE nid = %d', $edit->nid));
    db_query("UPDATE {node_comment_statistics} SET comment_count = %d, last_comment_timestamp = %d, last_comment_name = '%s', last_comment_uid = %d WHERE nid = %d", $count, time(), $user->name, $user->uid, $edit->nid);

    node_save($node);
  }

  return $edit->cid;
}

/**
 * Form API callback for previewing a project comment.
 *
 * @param $form
 *   The form to add the preview information to.
 * @param $edit
 *   The form values for the comment to preview.
 *
 * @return
 *   The modified form to render.
 *
 */
function project_comment_form_add_preview($form, $edit) {
  drupal_set_title(t('Preview comment'));
  if (is_array($edit)) {
    $comment = (object)$edit;
  }
  else {
    $comment = $edit;
  }
  project_comment_validate($comment);

  // Preview the comment with security check.
  if (!form_get_errors()) {
    $output = _project_comment_view_single($comment, 0);
  }

  $form['comment_preview'] = array(
    '#value' => $output,
    '#weight' => -100,
    '#prefix' => '<div class="preview"><div class="comment">',
    '#suffix' => '</div></div>',
  );

  $output = '';
  if (is_numeric(arg(3))) {
    $node = node_load(array('nid' => arg(3), 'type' => 'project_issue'));
    $output .= node_view($node, NULL, TRUE);
    $form['comment_preview_below'] = array('#value' => $output, '#weight' => 100);
  }
  unset($form['#sorted']);
  return $form;
}
