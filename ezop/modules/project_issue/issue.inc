<?php
// $Id: issue.inc,v 1.255.2.14 2008/01/30 22:36:47 thehunmonkgroup Exp $
// $Name: DRUPAL-5--1-3 $

function project_issue_page() {
  global $user;

  switch (!empty($_POST['op']) ? $_POST['op'] : arg(2)) {
    case 'rss':
      $project = project_project_retrieve(arg(3));
      if ($project->nid && node_access('view', $project)) {
        $query = new StdClass();
        $query->projects = array($project->nid);

        project_issue_query_result($query, 'rss');
      }
      else {
        project_issue_query_result(NULL, 'rss');
      }
      break;
    case 'add':
      $project = project_project_retrieve(arg(3));
      // We assume that a user may create issues for projects that he may view.
      if ($project->nid && node_access('view', $project)) {
        drupal_goto("node/add/project-issue/$project->uri");
      }
      else {
        drupal_goto("node/add/project-issue");
      }
      break;
    case 'statistics':
      $project = project_project_retrieve(arg(3));
      if ($project->nid && node_access('view', $project)) {
        return project_issue_statistics($project);
      }
      else {
        return project_issue_statistics();
      }
      break;
    case t('Subscribe'):
    case 'subscribe':
    case 'subscribe-mail':
      if ($user->uid) {
        if (valid_email_address($user->mail)) {
          $project = project_project_retrieve(arg(3));
          if ($project->nid && node_access('view', $project)) {
            return drupal_get_form('project_issue_subscribe', $project);
          }
          else {
            return drupal_get_form('project_issue_subscribe');
          }
        }
        else {
          return t('You must have a valid e-mail address in order to subscribe to issues. Please <a href="@update">update your account</a>.', array('@update' => url('user/'. $user->uid .'/edit', drupal_get_destination())));
        }
      }
      else {
        $destination = drupal_get_destination();
        return t('Please <a href="@login">login</a> or <a href="@register">register</a> in order to subscribe to issues.', array('@login' => url('user/login', $destination), '@register' => url('user/register', $destination)));
      }
      break;
    case 'user':
       $_GET['participated'] = $GLOBALS['user']->uid;
    default:
      $project = project_issue_build_form_url(arg(2));
      if ($project->nid && node_access('view', $project)) {
        $query = new StdClass();
        $query->projects = array($project->nid);

        return project_issue_query_result($query);
      }
      else {
        return project_issue_query_result();
      }
  }
}

/**
 * JS callback method to return updated elements on the issue form
 * when someone changes the "Project" selector. See project_issue.js.
 */
function project_issue_update_project($pid, $cid = NULL, $rid = NULL) {

  // Prevent a malicious user from snooping this callback directly.
  $error = array('error' => t('Malicious attempt to access unauthorized information detected.'));
  $node = node_load($pid);
  if ($node) {
    if (!node_access('view', $node)) {
      print drupal_to_js($error);
      exit();
    }
  }
  else {
    print drupal_to_js($error);
    exit();
  }

  // Release id options ("Version" selector).
  $return['rid'] = '';

  // Only generate release stuff if the project_release module is enabled.
  if (module_exists('project_release')) {
    $project->nid = $pid;
    if ($releases = project_release_get_releases($project, 0)) {
      $releases = array(t('<none>')) + $releases;
      // Since the value of releases is by nid, try to match release
      // based on the label instead.
      $default = 0;
      foreach ($releases as $key => $label) {
        if ($rid == $label) {
          $default = $key;
        }
      }
      $form = array();
      $form['rid'] = array(
        '#type' => 'select',
        '#title' => t('Version'),
        '#default_value' => $default,
        '#options' => $releases,
        '#required' => TRUE,
      );

      // Build the HTML output for the rid select.
      $form = form_builder('rid', $form);
      $return['rid'] .= drupal_render($form);
    }
  }

  // Components.
  $return['component'] = '';
  $project = db_fetch_object(db_query('SELECT * FROM {project_issue_projects} WHERE nid = %d', $pid));
  $components = array();
  if ($project->components) {
    $components = array(t('<none>'));
    foreach (unserialize($project->components) as $component) {
      $component = check_plain($component);
      $components[$component] = $component;
    }
  }

  $form = array();
  $form['component'] = array(
    '#type' => 'select',
    '#title' => t('Component'),
    '#default_value' => $cid,
    '#options' => $components,
    '#required' => TRUE,
  );

  // Build the HTML output for the component select.
  $form = form_builder('component', $form);
  $return['component'] .= drupal_render($form);

  // Set proper content type header for the AJAX call.
  drupal_set_header('Content-type: text/javascript');

  // Translate to js before output, so the calling js can work with the data.
  print drupal_to_js($return);
  exit();
}

/**
 * Decides what to do with search page request.
 *
 * - if this is not a POST, just return project_project_retrieve()
 * - if this is a POST, dispatch to handler function based on form id
 *
 * @param $url_arg
 *   Value returned from arg(2)
 */
function project_issue_build_form_url($url_arg) {
  $adv_filters = array('text', 'attachment', 'components', 'submitted', 'assigned', 'versions', 'participated', 'projects', 'states', 'categories', 'priorities');
  $filters = array_slice($adv_filters, 6);
  // Remember this is a request for "my issues"
  if ($url_arg == 'user') {
    $_REQUEST['participated'] = $GLOBALS['user']->uid;
  }

  // this isn't a POST, just return project
  if (empty($_POST)) {
    if ($url_arg == 'user') {
      // If this is the "my issues" page, don't query the DB for a
      // project that matches "user", just return and display all
      // matching issues regardless of project.
      return;
    }
    else {
      return project_project_retrieve($url_arg);
    }
  }

  // dispatch to hanlders based on value of $_POST['form_id']
  if ($_POST['form_id'] == 'project_issue_query_result_quick_search') {
    return project_issue_quick_search($url_arg, $filters);
  }
  else if ($_POST['form_id'] == 'project_issue_query') {
    return project_issue_advanced_search($url_arg, $adv_filters);
  }
}

/**
 * Handles quick search form
 *
 * @param $url_arg
 *   Value returned from arg(2)
 * @param $filters
 *   Search filters
 */
function project_issue_quick_search($url_arg, $filters) {
  if ($url_arg == 'user') {
    // My issues query, so stay on the same page
    $destination = 'project/issues/user';
  }
  elseif ($_POST['projects'] == 0 || strpos($_POST['projects'], ',') !== FALSE) {
    // search is for all/multiple projects, so just send to project/issues
    $destination = 'project/issues';
  }
  else {
    // try to find the project, and 404 if not found
    if (!$project = project_project_retrieve($_POST['projects'])) {
      drupal_not_found();
      exit;
    }
    $destination = 'project/issues/'. $project->uri;
  }

  // if 'q' is the only thing in $_GET, and it == destination, don't redirect
  if (count($_GET) == 1 && isset($_GET['q']) && $_GET['q'] == $destination) {

    // put $_POST where pager will see them
    foreach ($filters as $filter) {
      if (!empty($_POST[$filter])) {
        $_REQUEST[$filter] = $_POST[$filter];
      }
    }

    // unset $_REQUEST['edit'] so the pager links aren't full of cruft
    unset($_REQUEST['edit']);
    unset($_REQUEST['op']);
    return isset($project) ? $project : NULL;
  }

  // redirecting, so put filters in session
  foreach ($filters as $filter) {
    if (!empty($_POST[$filter])) {
      $issue_filters[$filter] = $_POST[$filter];
    }
  }
  drupal_goto($destination, drupal_query_string_encode($issue_filters));
}

/**
 * handles the advanced search form
 */
function project_issue_advanced_search($url_arg, $filters) {
  // Figure out what issue filters are in place for the query we're showing
  foreach ($filters as $filter) {
    if (!empty($_POST[$filter])) {
      if (is_array($_POST[$filter])) {
        $issue_filters[$filter] = implode(',', $_POST[$filter]);
      }
      else {
        $issue_filters[$filter] = $_POST[$filter];
      }
    }
  }

  // do we have just one project to search for?
  if (isset($_POST['projects']) && count($_POST['projects']) == 1) {
    // try to find the project, and 404 if not found
    if (!$project = project_project_retrieve($_POST['projects'][0])) {
      drupal_not_found();
      exit;
    }
    $destination = 'project/issues/'. $project->uri;
    drupal_goto($destination, drupal_query_string_encode($issue_filters));
  }

  // Put the issue filters where the pager links can find them.
  if (isset($issue_filters)) {
    foreach ($issue_filters as $name => $val) {
      $_REQUEST[$name] = $val;
    }
  }
  // unset $_REQUEST['edit'] so the pager links aren't full of cruft
  unset($_REQUEST['edit']);
  // Get rid of the silly 'op', since it's not a filter.
  unset($_REQUEST['op']);

  return project_project_retrieve($url_arg);
}

function project_issue_statistics($project = 0) {
  if ($project->nid) {
    $filter = sprintf(' AND p.pid = %d ', (int)$project->nid);
    project_project_set_breadcrumb($project, TRUE);
  }

  $output = '<div class="project-issue">';

  // Issue lifetime
  $header = array(t('Category'), t('Overall'), t('Last month'));
  $rows = array();
  $duration = time() - 30 * 24 * 60 * 60;
  $result = db_query(db_rewrite_sql('SELECT p.category, SUM(n.changed - n.created) / COUNT(p.category) AS duration FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND p.sid > 1 GROUP BY p.category'), $filter);
  while ($stat = db_fetch_object($result)) {
    $i++;
    $rows[$i][0] = project_issue_category($stat->category);
    $rows[$i][1] = array('data' => format_interval($stat->duration, 2), 'class' => 'numeric');
  }

  $result = db_query(db_rewrite_sql('SELECT p.category, SUM(n.changed - n.created) / COUNT(p.category) AS duration FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND p.sid > 1 AND n.created > %d GROUP BY p.category'), $filter, $duration);
  while ($stat = db_fetch_object($result)) {
    $j++;
    $rows[$j][2] = array('data' => format_interval($stat->duration, 2), 'class' => 'numeric');
  }
  $output .= '<h2>'. t('Average lifetime') .'</h2>';
  $output .= theme('table', $header, $rows);

  $header = array(t('Status'), t('Overall'), '%', t('Last month'));
  $rows = array();
  // Activity overall
  $total = db_result(db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s', 'p'), $filter));
  $result = db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total, p.sid FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s GROUP BY p.sid', 'p'), $filter);
  while ($stat = db_fetch_object($result)) {
    $rows[$stat->sid][0] = project_issue_state($stat->sid);;
    $rows[$stat->sid][1] = array('data' => $stat->total, 'class' => 'project-issue-numeric');
    $rows[$stat->sid][2] = array('data' => number_format($stat->total / $total * 100) .'%', 'class' => 'project-issue-numeric-light');
  }
  // Activity this week
  $result = db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total, p.sid FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND n.changed > %d GROUP BY p.sid', 'p'), $filter, $duration);
  while ($stat = db_fetch_object($result)) {
    $rows[$stat->sid][3] = array('data' => $stat->total, 'class' => 'project-issue-numeric');
  }
  $output .= '<h2>'. t('Issue activity') .'</h2>';
  $output .= theme('table', $header, $rows);

  // Project overview
  if (!$project->nid) {
    $header = array_merge(array(t('Project')), project_issue_state(), array(t('Total')));
    $rows = array();
    $result = db_query(db_rewrite_sql("SELECT pn.nid, pn.title, p.sid, COUNT(n.nid) AS total FROM {node} n INNER JOIN {project_issues} p ON n.nid = p.nid INNER JOIN {node} pn ON p.pid = pn.nid WHERE n.status = 1 AND pn.status = 1 GROUP BY pn.nid, p.sid, pn.title"));
    $orig = array('project' => array('data' => 0));
    foreach (project_issue_state() as $key => $value) {
      $orig[$key] = array('data' => '', 'class' => 'project-issue-numeric');
    }
    $orig['total'] = array('data' => '', 'class' => 'project-issue-numeric');

    while ($stat = db_fetch_object($result)) {
      if (!$rows[$stat->nid]['project']) {
        $rows[$stat->nid] = $orig;
        $rows[$stat->nid]['project']['data'] = l($stat->title, "node/$stat->nid");
      }
      $rows[$stat->nid]['total']['data'] += $stat->total;
      $rows[$stat->nid][$stat->sid]['data'] = $stat->total;
    }
    usort($rows, create_function('$a,$b', 'return $a[1]["data"] < $b[1]["data"];'));
    $output .= '<h2>'. t('Project overview') .'</h2>';
    $output .= theme('table', $header, $rows);
  }

  $output .= '</div>';
  return $output;
}

function project_issue_subscribe_submit($form_id, $form_values) {

  global $user;
  $all = $_POST['all'];

  $levels = array(0 => t('None'), 1 => t('Own issues'), 2 => t('All issues'));

    // Remove previous subscriptions for user.
    if ($nid = $form_values['single']) {
      db_query('DELETE FROM {project_subscriptions} WHERE nid = %d AND uid = %d', $nid, $user->uid);
    }
    else {
      db_query('DELETE FROM {project_subscriptions} WHERE uid = %d', $user->uid);
    }

    if ($all) {
      $_level = array_search($all, $levels);
    }

    foreach ($form_values['options'] as $nid => $level) {
      if ($_level !== 0 && $level !== 0) {
        db_query('INSERT INTO {project_subscriptions} (nid, uid, level) VALUES (%d, %d, %d)', $nid, $user->uid, $_level ? $_level : $level);
      }
    }
    drupal_set_message(t('Subscription settings saved.'));

    return 'project/issues/subscribe-mail';
}


function project_issue_subscribe($project = 0) {
  global $user;

  $levels = array(0 => t('None'), 1 => t('Own issues'), 2 => t('All issues'));

  if ($project) {
    $level = db_result(db_query('SELECT level FROM {project_subscriptions} WHERE nid = %d AND uid = %d', $project->nid, $user->uid));
    $form['single'] = array(
      '#type' => 'value',
      '#value' => $project->nid,
    );
    $form['subscribe'] = array(
      '#type' => 'markup',
      '#value' => '<p>'. t('Subscribe to receive e-mail notification when an issue for this project is updated.') .'</p>',
    );
    $form['options']['#tree'] = TRUE;
    $form['options'][$project->nid] = array(
      '#type' => 'radios',
      '#title' => t('Subscribe to @project issues', array('@project' => $project->title)),
      '#default_value' => isset($level) ? $level : 0,
      '#options' => $levels,
    );

  }
  else {

    $form['buttons']['all'] = array(
      '#type' => 'markup',
      '#value' => t('All projects'),
    );
    foreach ($levels as $key => $level) {
      $form['buttons'][$level] = array(
        '#type' => 'submit',
        '#name' => 'all',
        '#value' => $level,
      );
    }
    $nids = array();

    $result = db_query(db_rewrite_sql("SELECT s.nid, n.title, s.level, p.uri FROM {project_subscriptions} s INNER JOIN {node} n ON n.nid = s.nid INNER JOIN {project_projects} p ON n.nid = p.nid WHERE n.type = 'project_project' AND n.status = 1 AND s.uid = %d ORDER BY n.title", 's'), $user->uid);
    while ($project = db_fetch_object($result)) {
      $form['project'][$project->nid]['title'] = array(
        '#value' => l($project->title, "project/$project->uri"),
      );
      foreach ($levels as $key => $level) {
        if ($project->level == $key) {
          $status[$project->nid] = $key;
        }
      }
      $nids[] = $project->nid;
    }

    if (empty($nids)) {
      $placeholders = '';
    }
    else {
      $placeholders = " AND n.nid NOT IN (". implode(',', array_fill(0, count($nids), '%d')) .")";
    }

    $result = db_query(db_rewrite_sql("SELECT n.nid, n.title, p.uri FROM {node} n INNER JOIN {project_projects} p ON n.nid = p.nid WHERE n.type = 'project_project' AND n.status = 1". ($nids ?  $placeholders : "") ." ORDER BY n.title"), $nids);
    while ($project = db_fetch_object($result)) {
      $form['project'][$project->nid]['title'] = array(
        '#value' => l($project->title, "project/$project->uri"),
      );
      $nids[] = $project->nid;
    }

    foreach ($nids as $nid) {
      $form['options']['#tree'] = TRUE;
      $form['options'][$nid] = array(
        '#type' => 'radios',
        '#default_value' => isset($status[$nid]) ? $status[$nid] : 0,
        '#options' => $levels,
      );
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Subscribe'),
  );
  return $form;
}

function theme_project_issue_subscribe($form) {
  global $user;

  $output = '';

  if (!$form['single']) {
    $levels = array(0 => t('None'), 1 => t('Own issues'), 2 => t('All issues'));
    $headers = array_merge(array(t('Project')), $levels);

    $row = array();
    foreach (element_children($form['buttons']) as $key) {
      $row[] = drupal_render($form['buttons'][$key]);
    }
    $rows = array($row);

    foreach (element_children($form['project']) as $key) {
      $row = array(drupal_render($form['project'][$key]['title']));
      foreach ($levels as $level => $name) {
        $row[] = drupal_render($form['options'][$key][$level]);
      }
      $rows[] = $row;
    }
    $output = theme('table', $headers, $rows);
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * Implementation of hook_form().
 */
function project_issue_form($node, $param) {
  global $user;

  $default_state = variable_get('project_issue_default_state', 1);
  // Set form parameters so we can accept file uploads.
  $form['#attributes'] = array('enctype' => 'multipart/form-data');

  // Set the pre-render function for adjustments to the form stages
  $form['#pre_render'] = array('project_issue_form_pre_render');

  $form['page'] = array(
    '#type' => 'hidden',
    '#value' => isset($_POST['page']) ? $_POST['page'] : 1,
  );

  $form['#prefix'] = '<div class="project-issue">';
  $form['#suffix'] = '</div>';

  // Fetch a list of all projects to make swapping simpler
  $uris = NULL;
  $projects = array(t('<none>')) + project_projects_select_options($uris);
  if (count($projects) == 1) {
    drupal_set_message(t('You do not have access to any projects.'), 'error');
  }

  // Try to find the active project, if not already known.
  if (empty($node->pid)) {
    if (isset($_POST['pid'])) {
      $pid = $_POST['pid'];
    }
    else {
      $pid = arg(3);
    }
    if (isset($pid)) {
      if (is_numeric($pid)) {
        $node->pid = db_result(db_query(db_rewrite_sql('SELECT p.nid FROM {project_projects} p WHERE p.nid = %d', 'p'), $pid), 0);
      }
      else {
        $node->pid = db_result(db_query(db_rewrite_sql("SELECT p.nid FROM {project_projects} p WHERE p.uri = '%s'", 'p'), $pid), 0);
      }
    }
  }
  $pid = $node->pid;
  if ($pid) {
    // Load Javascript. This is needed only if we already have a project selected.
    drupal_add_js(drupal_get_path('module', 'project_issue') .'/project_issue.js');
    drupal_add_js(array('projectUrl' => url('project/issues/update_project')), 'setting');

    // Load the project and initialize some support arrays.
    $project = node_load(array('nid' => $pid, 'type' => 'project_project'));
    if (trim($project->help) && empty($_POST)) {
      drupal_set_message(filter_xss($project->help));
    }
    if (module_exists('project_release') &&
        $releases = project_release_get_releases($project, 0)) {
      $releases = array(t('<none>')) + $releases;
    }
    $components = array();
    if ($project->components) {
      $components = array(t('<none>'));
      foreach ($project->components as $component) {
        $component = check_plain($component);
        $components[$component] = $component;
      }
    }
    $categories = array_merge(array(t('<none>')), project_issue_category(0, 0));
    $priorities = project_issue_priority();
    $states = project_issue_state(0, true, $node->nid && ($node->uid == $user->uid), $node->sid);

    if ($user->uid == $node->assigned) {
      $assigned = array(0 => t('Unassign'), $user->uid => $user->name);
    }
    else {
      $assigned = array(
        $node->assigned => ($node->assigned && ($account = user_load(array('uid' => $node->assigned))) ? $account->name : t('Unassigned')),
        $user->uid => $user->name
      );
    }

    $form['project_info'] = array(
      '#type' => 'fieldset',
      '#title' => t('Project information'),
      '#prefix' => '<div class="inline-options">',
      '#suffix' => '</div>',
    );
    $form['project_info']['pid'] = array(
      '#type' => 'select',
      '#title' => t('Project'),
      '#default_value' => $node->pid,
      '#options' => $projects,
      '#required' => TRUE,
    );
    if ($releases) {
      $form['project_info']['rid'] = array(
        '#type' => 'select',
        '#title' => t('Version'),
        '#default_value' => $node->rid,
        '#options' => $releases,
      );
    }
    $form['project_info']['component'] = array(
      '#type' => 'select',
      '#title' => t('Component'),
      '#default_value' => $node->component,
      '#options' => $components,
    );

    $form['issue_info'] = array(
      '#type' => 'fieldset',
      '#title' => t('Issue information'),
      '#prefix' => '<div class="inline-options">',
      '#suffix' => '</div>',
    );
    $form['issue_info']['category'] = array(
      '#type' => 'select',
      '#title' => t('Category'),
      '#default_value' => $node->category ? $node->category : arg(4),
      '#options' => $categories,
    );
    $form['issue_info']['priority'] = array(
      '#type' => 'select',
      '#title' => t('Priority'),
      '#default_value' => $node->priority ? $node->priority : 2,
      '#options' => $priorities,
    );
    $form['issue_info']['assigned'] = array(
      '#type' => 'select',
      '#title' => t('Assigned'),
      '#default_value' => $node->assigned,
      '#options' => $assigned,
    );
    if (count($states) > 1) {
      $form['issue_info']['sid'] = array(
        '#type' => 'select',
        '#title' => t('Status'),
        '#default_value' => $node->sid ? $node->sid : $default_state,
        '#options' => $states,
      );
    }
    else {
      $form['issue_info']['sid'] = array(
        '#type' => 'hidden',
        '#value' => $default_state,
      );
      $form['issue_info']['status'] = array(
        '#type' => 'item',
        '#title' => t('Status'),
        '#value' => project_issue_state($default_state),
      );
    }

    $form['issue_details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Issue details'),
      '#prefix' => '</div><div class="standard">',
    );
    $form['issue_details']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#default_value' => $node->title,
      '#size' => 60,
      '#maxlength' => 128,
    );
    if ($node->title) {
      $form['issue_details']['title']['#default_value'] = $node->title;
      $form['issue_details']['title']['#description'] = t('Note: modifying this value will change the title of the entire issue, not your follow-up comment.');
    }
    $form['issue_details']['body'] = array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#default_value' => $node->body,
      '#rows' => 10,
    );
    $form['issue_details']['format'] = filter_form($node->format);

    $directory = file_create_path(variable_get('project_directory_issues', 'issues'));
    if (file_check_directory($directory, 0)) {
      $file = file_check_upload('file_issue');
      $form['issue_details']['file_issue'] = array(
        '#type' => 'file',
        '#title' => t('File attachment'),
        '#size' => 40,
        '#description' => ($file ? t('A file named %filename has already been uploaded. If you upload another file %filename will be replaced.', array('%filename' => $file->filename)) : t('Optionally attach a file, for example a patch or a screenshot.')),
      );
    }
    else {
      $msg = t('File attachments are disabled. The issue directory has not been properly configured.');
      if (user_access('administer site configuration')) {
        $msg .= ' '. t('Please visit the !admin-project-issue-settings page.', array('!admin-project-issue-settings' => l(t('Project issue settings'), 'admin/project/project-issue-settings')));
      }
      else {
        $msg .= ' '. t('Please contact the site administrator.');
      }
      drupal_set_message($msg, 'warning');
    }
  }
  else {
    $form['pid'] = array(
      '#type' => 'select',
      '#title' => t('Project'),
      '#options' => $projects,
      '#required' => TRUE,
    );

  }

  return $form;
}

function project_issue_form_pre_render($form_id, &$form) {
  if ($form_id == 'project_issue_node_form') {

    // Only move to the next page if there were no form errors.
    // We don't want to keep incrementing page forever, since users
    // can successfully preview many times.  However, if we already
    // have a value for 'Project' (via a submit with project in the
    // URL), we'll already be on page 2 of the form and must increment
    // our page or we'll display the wrong buttons.
    if (!form_get_errors() && $form['page']['#value'] < 2 && (isset($_POST['page']) || isset($form['project_info']['pid']['#value']))) {
      $form['page']['#value'] = $form['page']['#value'] + 1;
    }

    if ($form['page']['#value'] == 1) {
      // If we're on the first page, we want to convert 'preview' into
      // a 'next' button and hide 'submit'.
      $form['preview'] = array(
        '#type' => 'button',
        '#value' => t('Next'),
        '#weight' => 19,
      );
      $form['submit'] = array(
        '#type' => 'value',
        '#value' => 'hidden',
      );
    }
    else {
      /*
        We're on page 2.  In this case, we want all the default
        buttons.  Furthermore, we must now set the #required attribute
        on all the fields that are now in the form (which are only
        added once we know the project from page #1 or from a direct
        link), so that when we now attempt to validate the input (and
        display the form), these fields will be required.
      */
      _project_issue_form_add_required_fields($form, TRUE);
    }
  }
}

/**
 * Private helper method to set the '#required' attribute to TRUE for
 * all the fields that should be required on the issue form.  These
 * fields can't be set to required from project_issue_form() because
 * this is a 2 page form, and so the FAPI validation code would mark
 * them all as an error as soon as the user lands on page 2.
 * Therefore, they're set to required by the pre_render hook (after
 * initially validating the page, but before it is displayed).  This
 * This is a separate method so that it can also be called from
 * comment.inc, which shares the project_issue_form() code, but
 * doesn't use it as a 2-page form, so we want these fields to be
 * required right away in that case.
 *
 * @param $form
 *   Reference to the form to modify
 * @param $needs_body
 *   Boolean that specifies if the 'body' field should be required
 */
function _project_issue_form_add_required_fields(&$form, $needs_body) {
  $form['project_info']['rid']['#required'] = TRUE;
  $form['project_info']['component']['#required'] = TRUE;
  $form['issue_info']['category']['#required'] = TRUE;
  $form['issue_details']['title']['#required'] = TRUE;
  if ($needs_body) {
    $form['issue_details']['body']['#required'] = TRUE;
  }
}

function project_issue_node_form_validate($form_id, $form_values, $form) {

  if (!$form_values['pid']) {
    form_set_error('pid', t('You have to specify a valid project.'));
  }

  if ($form_values['page'] == 2) {
    if ($form_values['pid'] && $project = node_load($form_values['pid'])) {
      $node->title = $form_values['title'];
      if (module_exists('project_release') &&
          $releases = project_release_get_releases($project, 0)) {
        empty($form_values['rid']) and form_set_error('rid', t('You have to specify a valid version.'));
      }
      if ($form_values['component'] && !in_array($form_values['component'], $project->components)) {
        $form_values['component'] = 0;
      }
      empty($form_values['component']) && form_set_error('component', t('You have to specify a valid component.'));
      empty($form_values['category']) && form_set_error('category', t('You have to specify a valid category.'));
      empty($form_values['title']) && form_set_error('title', t('You have to specify a valid title.'));
      empty($form_values['body']) && form_set_error('body', t('You have to specify a valid description.'));

      if ($file = file_check_upload('file_issue')) {
        if (project_issue_validate_file($file)) {
          $form_values['file'] = file_save_upload($file);
        }
      }
    }
    node_form_validate($form_id, $form_values, $form);
  }
}

function project_issue_comment_validate(&$node) {

  // Set default values.
  empty($node->priority) and $node->priority = 2;
  empty($node->category) and $node->category = arg(4);
  empty($node->sid) and $node->sid = variable_get('project_issue_default_state', 1);

  // Try to find the active project
  if (empty($node->pid)) {
    if (isset($_POST['pid'])) {
      $pid = $_POST['pid'];
    }
    else {
      $pid = arg(3);
    }
    if (isset($pid)) {
      if (is_numeric($pid)) {
        $node->pid = db_result(db_query(db_rewrite_sql('SELECT p.nid FROM {project_projects} p WHERE p.nid = %d', 'p'), $pid), 0);
      }
      else {
        $node->pid = db_result(db_query(db_rewrite_sql("SELECT p.nid FROM {project_projects} p WHERE p.uri = '%s'", 'p'), $pid), 0);
      }
    }
  }

  // Validate the rest of the form.
  if (isset($node->title) && !$node->validated) {
    if ($node->pid && $project = node_load($node->pid)) {
      if (module_exists('project_release') &&
          $releases = project_release_get_releases($project, 0)) {
        empty($node->rid) and form_set_error('rid', t('You have to specify a valid version.'));
      }
      else {
        // If there's no version defined, set this to 0 to match the
        // value stored in the issue. Otherwise, project_comment_save()
        // will think we've changed the version with this reply and
        // will store extra data in the DB we don't want or need.
        $node->rid = 0;
      }
      if ($node->component && !in_array($node->component, $project->components)) {
        $node->component = 0;
      }

      empty($node->component) and form_set_error('component', t('You have to specify a valid component.'));
      empty($node->category) and form_set_error('category', t('You have to specify a valid category.'));

      // make sure this followup changes something or has a body
      if (!empty($_POST) && empty($node->body)) {
        if (is_numeric(arg(3))) {
          $issue = node_load(arg(3));
        }
        if (isset($issue)
            && $node->pid == $issue->pid
            && $node->rid == $issue->rid
            && $node->component == $issue->component
            && $node->category == $issue->category
            && $node->priority == $issue->priority
            && $node->assigned == $issue->assigned
            && $node->sid == $issue->sid
            && $node->title == $issue->title) {
          form_set_error('body', t('You must either specify a description or change something about this issue.'));
        }
      }

      $file = file_check_upload('file_issue');
      if ($file && project_issue_validate_file($file)) {
        $node->file = file_save_upload($file);
      }
    }
    elseif (isset($node->pid)) {
      form_set_error('pid', t('You have to specify a valid project.'));
    }
  }

  return $node;
}

/**
 * Ensure only files with allowed extension are uploaded.
 */
function project_issue_validate_file($file) {
  $whitelist = project_issue_filename_extension_whitelist();
  $matches = array();
  if (preg_match('/\.([^.]*)$/', $file->filename, $matches)) {
    if (in_array($matches[1], $whitelist)) {
      return TRUE;
    }
  }
  form_set_error('file_issue', t('The selected file %name can not be attached to this post, because it is only possible to attach files with the following extensions: %files-allowed.', array('%name' => $file->filename, '%files-allowed' => implode(', ', $whitelist))));
  return FALSE;
}

function project_issue_view($node, $teaser = false, $page = false) {
  $node = node_prepare($node, $teaser);

  if (!$teaser && $page) {

    $node->content['#prefix'] = '<div class="project-issue">';
    $node->content['#suffix'] = '</div>';

    $project = node_load(array('nid' => $node->pid, 'type' => 'project_project'));
    $release->nid = $node->rid;
    if (module_exists('project_release')) {
      $release = project_release_load($release);
    }
    $assigned = ($node->assigned && ($account = user_load(array('uid' => $node->assigned))) ? $account->name : t('Unassigned'));

    $rows = array();
    $rows[] = array('Project:', check_plain($project->title));
    if ($release->version) {
      $rows[] = array('Version:', check_plain($release->version));
    }
    $rows[] = array(t('Component:'),  check_plain($node->component));
    $rows[] = array(t('Category:'), project_issue_category($node->category, 0));
    $rows[] = array(t('Priority:'), project_issue_priority($node->priority));
    $rows[] = array(t('Assigned:'), $assigned);
    $rows[] = array(t('Status:'), project_issue_state($node->sid));
    if ($node->file_path && file_exists($node->file_path)) {
      $rows[] = array(t('Attachment:'), '<a href="'. file_create_url($node->file_path). '">'. basename($node->file_path) .'</a> ('. format_size($node->file_size) .')');
    }
    $node->content['project_issue_summary'] = array(
      '#value' => '<div class="summary">'. theme('table', array(), $rows) .'</div>',
      '#weight' => -5,
    );
    $node->content['project_issue_header'] = array(
      '#value' => '<div class="header">'. t('Description') .'</div>',
      '#weight' => -3,
    );
    $node->content['project_issue_comments'] = array(
      '#value' => project_comment_view($node),
      '#weight' => 2,
    );

    project_issue_set_breadcrumb($node, $project);
  }
  return $node;
}

function project_issue_load($node) {
  $project = db_fetch_object(db_query(db_rewrite_sql('SELECT pi.* FROM {project_issues} pi WHERE pi.nid = %d', 'pi'), $node->nid));
  return $project;
}

function project_issue_insert($node) {
  if ($node->file) {
    $directory = file_create_path(variable_get('project_directory_issues', 'issues'));
    $node->file->filename = project_issue_munge_filename($node->file->filename);
    $file = file_save_upload($node->file, $directory);
  }
  db_query("INSERT INTO {project_issues} (nid, pid, category, component, priority, rid, assigned, sid, file_path, file_mime, file_size) VALUES (%d, %d, '%s', '%s', %d, %d, %d, %d, '%s', '%s', %d)", $node->nid, $node->pid, $node->category, $node->component, $node->priority, $node->rid, $node->assigned, $node->sid, $file->filepath, $file->filemime, $file->filesize);
  project_mail_notify($node);
}

function project_issue_update($node) {
  if ($node->file) {
    // Remove old file.
    file_delete(db_result(db_query('SELECT file_path FROM {project_issues} WHERE nid = %d', $node->nid)));
    $file = file_save_upload($node->file, variable_get('project_directory_issues', 'issues'));
    db_query("UPDATE {project_issues} SET pid = %d, category = '%s', component = '%s', priority = %d, rid = %d, assigned = %d, sid = %d, file_path = '%s', file_mime = '%s', file_size = %d WHERE nid = %d", $node->pid, $node->category, $node->component, $node->priority, $node->rid, $node->assigned, $node->sid, $file->filepath, $file->filemime, $file->filesize, $node->nid);
  }
  else {
    db_query("UPDATE {project_issues} SET pid = %d, category = '%s', component = '%s', priority = %d, rid = %d, assigned = %d, sid = %d WHERE nid = %d", $node->pid, $node->category, $node->component, $node->priority, $node->rid, $node->assigned, $node->sid, $node->nid);
  }
  project_mail_notify($node);
}

function project_issue_delete($node) {
  file_delete($node->file_path);
  $comment_files = db_query("SELECT file_path FROM {project_comments} WHERE file_path <> '' AND nid = %d", $node->nid);
  while ($comment = db_fetch_object($comment_files)) {
    file_delete($comment->file_path);
  }
  db_query('DELETE FROM {project_issues} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {project_comments} WHERE nid = %d', $node->nid);
}

function project_issue_access($op, $node) {
  global $user;
  if (user_access('administer projects')) {
    return TRUE;
  }
  switch ($op) {
    case 'view':
      if (user_access('access own project issues') && $node->uid == $user->uid) {
        return TRUE;
      }
      if (!user_access('access project issues')) {
        return FALSE;
      }
      break;
    case 'create':
      return user_access('create project issues');
    case 'update':
      if (user_access('edit own project issues') && $node->uid == $user->uid) {
        return TRUE;
      }
      break;
    case 'delete':
      // Admin case already handled, no one else should be able to delete.
      break;
  }
}

// Support stuff

/**
 * Return information about project issue state values.
 *
 * @param $sid
 *   Integer state id to return information about, or 0 for all states.
 * @param $restrict
 *   Boolean to determine if states should be restricted based on the
 *   permissions of the current user or $account if that parameter
 *   is provided.
 * @param $is_author
 *   Boolean that indicates if the current user is the author of the
 *   issue, which can potentially grant a wider selection of states.
 * @param $current_sid
 *   The current integer state id for the issue.
 * @param $defaults
 *   Boolean to request the states used for default issue queries.
 * @param $account
 *   Account of a user to pass to user_access() for access checking.
 *   This parameter will have no effect unless $restrict is also
 *   set to TRUE.
 *
 * @return
 *   An array of states (sid as key, name as value) that match the
 *   given filters, or the name of the requested state.
 */
function project_issue_state($sid = 0, $restrict = false, $is_author = false, $current_sid = 0, $defaults = false, $account = NULL) {
  static $options;

  if (!$options) {
    $result = db_query('SELECT * FROM {project_issue_state} ORDER BY weight');
    while ($state = db_fetch_object($result)) {
      $options[] = $state;
    }
  }

  foreach($options as $state) {
    if ($restrict) {
      // Check if user has access,
      // or if status is default status and therefore available to all,
      // or if user is original issue author and author has access,
      // or if the issue is already in a state, even if the user doesn't have
      //   access to that state themselves.
      if (user_access('set issue status '. str_replace("'", "", $state->name), $account)
          || user_access('administer projects', $account)
          || ($state->sid == variable_get('project_issue_default_state', 1))
          || ($state->author_has && $is_author)
          || ($state->sid == $current_sid) ) {
        $states[$state->sid] = $state->name;
      }
    }
    else if ($defaults) {
      if ($state->default_query) {
        $states[$state->sid] = $state->name;
      }
    }
    else {
      $states[$state->sid] = $state->name;
    }
  }

  return $sid ? $states[$sid] : $states;
}

/**
 * Return an array of state ids that should be used for default queries.
 */
function project_issue_default_states() {
  static $defaults;
  if (empty($defaults)) {
    $states = project_issue_state(0, false, false, 0, true);
    $defaults = !empty($states) ? array_keys($states) : array(1, 2, 4);
  }
  return $defaults;
}

function project_issue_priority($priority = 0) {
  $priorities = array(1 => t('critical'), t('normal'), t('minor'));
  return $priority ? $priorities[$priority] : $priorities;
}

function project_issue_category($category = 0, $plural = 1) {
  if ($plural) {
    $categories = array('bug' => t('bug reports'), 'task' => t('tasks'), 'feature' => t('feature requests'), 'support' => t('support requests'));
  }
  else {
    $categories = array('bug' => t('bug report'), 'task' => t('task'), 'feature' => t('feature request'), 'support' => t('support request'));
  }
  return $category ? $categories[$category] : $categories;
}

function project_issue_count($pid) {
  $state = array();
  $result = db_query('SELECT p.sid, count(p.sid) AS count FROM {node} n INNER JOIN {project_issues} p ON n.nid = p.nid WHERE n.status = 1 AND p.pid = %d GROUP BY p.sid', $pid);
  while ($data = db_fetch_object($result)) {
    $state[$data->sid] = $data->count;
  }
  return $state;
}

function project_issue_search_page($project_name = NULL, $query = NULL) {
  $project = project_project_retrieve($project_name);
  if (isset($project) && $project->nid && node_access('view', $project)) {
    return drupal_get_form('project_issue_query', $project, $query);
  }
  else {
    return drupal_get_form('project_issue_query', 0, $query);
  }
}

function project_issue_query($project = 0, $query = NULL) {
  $categories = project_issue_category();
  $projects = array();
  $components = array();
  if (!empty($project->nid)) {
    drupal_set_title(t('Search issues for %name', array('%name' => $project->title)));
    project_project_set_breadcrumb($project, TRUE);
    foreach ($project->components as $component) {
      $component = check_plain($component);
      $components[$component] = $component;
    }
    if (module_exists('project_release')) {
      $versions = project_release_get_releases($project, 0);
    }
  }
  else {
    $uris = NULL;
    $projects = project_projects_select_options($uris);
    drupal_set_title(t('Search issues for all projects'));
  }
  if (is_null($query)) {
    $query = (object) array(
      'projects' => '',
      'summary' => '',
      'attachment' => '',
      'versions' => '',
      'components' => '',
      'categories' => '',
      'priorities' => '',
      'states' => '',
      'submitted' => '',
      'assigned' => '',
      'users' => '',
    );
  }
  $states = project_issue_state();
  $priorities = project_issue_priority();

  $form['#action'] = url(isset($project->uri) ? "project/issues/$project->uri" : 'project/issues');
  $form['text'] = array(
    '#type' => 'textfield',
    '#title' => t('Search for'),
    '#default_value' => $query->summary,
    '#maxlength' => 255,
  );
  $form['attachment'] = array(
    '#type' => 'checkbox',
    '#title' => t('Has attachment.'),
    '#default_value' => $query->attachment,
  );

  if ($projects) {
    $form['projects'] = array(
      '#type' => 'select',
      '#title' => t('Projects'),
      '#default_value' => $query->projects,
      '#options' => $projects,
      '#attributes' => array('size' => 5),
      '#multiple' => TRUE,
    );
    $form['categories'] = array(
      '#type' => 'select',
      '#title' => t('Categories'),
      '#default_value' => $query->categories,
      '#options' => $categories,
      '#attributes' => array('size' => 5),
      '#multiple' => TRUE,
    );
  }
  else {
    if (!empty($versions)) {
      $form['versions'] = array(
        '#type' => 'select',
        '#title' => t('Versions'),
        '#default_value' => $query->versions,
        '#options' => $versions,
        '#attributes' => array('size' => 5),
        '#multiple' => TRUE,
      );
    }
    $form['components'] = array(
      '#type' => 'select',
      '#title' => t('Components'),
      '#default_value' => $query->components,
      '#options' => $components,
      '#attributes' => array('size' => 5),
      '#multiple' => TRUE,
    );
    $form['categories'] = array(
      '#type' => 'select',
      '#title' => t('Categories'),
      '#default_value' => $query->categories,
      '#options' => $categories,
      '#attributes' => array('size' => 5),
      '#multiple' => TRUE,
    );
  }
  $form['priorities'] = array(
    '#type' => 'select',
    '#title' => t('Priorities'),
    '#default_value' => $query->priorities,
    '#options' => $priorities,
    '#attributes' => array('size' => 5),
    '#multiple' => TRUE,
  );
  $form['states'] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#options' => $states,
    '#default_value' => ($query->states) ? $query->states : project_issue_default_states(),
    '#attributes' => array('size' => 5),
    '#multiple' => TRUE,
  );
  $form['submitted'] = array(
    '#type' => 'textfield',
    '#title' => t('Submitted by'),
    '#default_value' => $query->submitted,
    '#autocomplete_path' => 'user/autocomplete',
    '#size' => 20,
    '#maxlength' => 255,
  );
  $form['assigned'] = array(
    '#type' => 'textfield',
    '#title' => t('Assigned'),
    '#default_value' => $query->assigned,
    '#autocomplete_path' => 'user/autocomplete',
    '#size' => 20,
    '#maxlength' => 255,
  );
  $form['participated'] = array(
    '#type' => 'textfield',
    '#title' => t('Participant'),
    '#default_value' => $query->users,
    '#autocomplete_path' => 'user/autocomplete',
    '#size' => 20,
    '#maxlength' => 255,
  );
  $form['submit'] = array(
    '#type' => 'button',
    '#value' => t('Search'),
  );
  return $form;
}

function theme_project_issue_query($form) {

  $rows[] = array(
    array('data' => drupal_render($form['text']), 'colspan' => 3)
  );
  $rows[] = array(
    array('data' => drupal_render($form['attachment']), 'colspan' => 3)
  );
  if (isset($form['projects'])) {
    $rows[] = array(
      drupal_render($form['projects']),
      drupal_render($form['categories']),
      NULL,
    );
  }
  elseif (isset($form['versions'])) {
    $rows[] = array(
      drupal_render($form['versions']),
      drupal_render($form['components']),
      drupal_render($form['categories']),
    );
  }
  else {
    $rows[] = array(
      array('data' => drupal_render($form['components'])),
      array('data' => drupal_render($form['categories']), 'colspan' => 2)
    );
  }
  $rows[] = array(
    array('data' => drupal_render($form['priorities'])),
    array('data' => drupal_render($form['states']), 'colspan' => 2)
  );
  $rows[] = array(
    drupal_render($form['submitted']),
    drupal_render($form['assigned']),
    drupal_render($form['participated'])
  );
  $rows[] = array(
    array('data' => drupal_render($form['submit']), 'colspan' => 3)
  );

  $output = '<div class="project-issue">';
  $output .= theme('table', array(), $rows);
  $output .= '</div>';
  $output .= drupal_render($form);

  return $output;
}

function theme_project_issue_admin_states_form($form) {
  $header = array(
    array('data' => t('ID')),
    array('data' => t('Name')),
    array('data' => t('Weight')),
    array('data' => t('Author may set')),
    array('data' => t('In default queries')),
    array('data' => t('Default status')),
    array('data' => t('Operations'))
  );
  foreach (element_children($form['status']) as $key) {
    $rows[] = array(
      drupal_render($form['status'][$key]['sid']),
      drupal_render($form['status'][$key]['name']),
      drupal_render($form['status'][$key]['weight']),
      drupal_render($form['status'][$key]['author_has']),
      drupal_render($form['status'][$key]['default_query']),
      drupal_render($form['default_state'][$key]),
      drupal_render($form['delete'][$key])
    );
  }
  $rows[] = array(
    NULL,
    drupal_render($form['status_add']['name']),
    drupal_render($form['status_add']['weight']),
    drupal_render($form['status_add']['author_has']),
    drupal_render($form['status_add']['default_query']),
    NULL, NULL,
  );
  $output = '<div>' . theme('table', $header, $rows) . '</div>';
  $output .= drupal_render($form);
  return $output;
}

function project_issue_admin_states_form() {
  $result = db_query('SELECT * FROM {project_issue_state} ORDER BY weight');
  $default_state = variable_get('project_issue_default_state', 1);
  $default_states = project_issue_default_states();
  $form['status']['#tree'] = TRUE;
  while ($state = db_fetch_object($result)) {
    $options[$state->sid] = '';
    $form['status'][$state->sid]['sid'] = array(
      '#type' => 'markup',
      '#value' => $state->sid,
    );
    $form['status'][$state->sid]['name'] = array(
      '#type' => 'textfield',
      '#default_value' => $state->name,
      '#size' => 20,
      '#maxlength' => 255,
    );
    $form['status'][$state->sid]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $state->weight,
      '#delta' => 15,
    );
    $form['status'][$state->sid]['author_has'] = array(
      '#type' => 'checkbox',
      '#default_value' => $state->author_has,
    );
    $form['status'][$state->sid]['default_query'] = array(
      '#type' => 'checkbox',
      '#default_value' => in_array($state->sid, $default_states),
    );
    $del_link = ($state->sid != $default_state) ? l(t('Delete'), 'admin/project/project-issue-status/delete/'. $state->sid) : '';
    $form['delete'][$state->sid] = array(
      '#type' => 'markup',
      '#value' => $del_link,
    );
  }
  $form['default_state'] = array(
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => $default_state,
  );
  $form['status_add']['name'] = array(
    '#type' => 'textfield',
    '#size' => 20,
    '#maxlength' => 255,
  );
  $form['status_add']['weight'] = array(
    '#type' => 'weight',
    '#default_value' => 0,
    '#delta' => 15,
  );
  $form['status_add']['author_has'] = array(
    '#type' => 'checkbox',
  );
  $form['status_add']['default_query'] = array(
    '#type' => 'checkbox',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#tree'] = TRUE;
  return $form;
}

/**
 * Submit handler for project_issue_admin_states_form.
 */
function project_issue_admin_states_form_submit($form_id, $form_values) {
  // Check for and apply changes or additions to project issue status options.
  if (isset($form_values['default_state'])) {
    variable_set('project_issue_default_state', $form_values['default_state']);
  }
  // Update existing status options.
  if($form_values['status']) {
    foreach ($form_values['status'] as $sid => $value) {
      $state = db_fetch_object(db_query('SELECT name, weight, author_has, default_query FROM {project_issue_state} WHERE sid = %d', $sid));
      // Check to see whether the record needs updating.
      if (($state->name != $value['name']) || ($state->weight != $value['weight']) || ($state->author_has != $value['author_has']) || ($state->default_query != $value['default_query'])) {
        db_query("UPDATE {project_issue_state} SET name = '%s', weight = %d, author_has = %d, default_query = %d WHERE sid = %d", $value['name'], $value['weight'], $value['author_has'], $value['default_query'], $sid);
      }
    }
  }
  // Add any new status options.
  if (isset($form_values['status_add']) && !empty($form_values['status_add']['name'])) {
    // Check to see whether the state already exists:
    $query = db_query("SELECT name FROM {project_issue_state} WHERE name = '%s'", $form_values['status_add']['name']);
    if (!db_num_rows($query)) {
      db_query("INSERT INTO {project_issue_state} (name, weight, author_has, default_query) VALUES ('%s', %d, %d, %d)", $form_values['status_add']['name'], $form_values['status_add']['weight'], $form_values['status_add']['author_has'], $form_values['status_add']['default_query']);
    }
    else {
      drupal_set_message(t('Status %status already exists.', array ('%status' => $form_values['status_add']['name'])));
    }
  }
}

function project_issue_delete_state_confirm($sid) {
  $sid = arg(4);
  $states = project_issue_state();
  $name = $states[$sid];

  $total = db_result(db_query('SELECT COUNT(nid) AS total FROM {project_issues} WHERE sid = %d', $sid));
  if ($total > 0) {
    $form['new_sid'] = array(
      '#type' => 'select',
      '#title' => t('Reassign status'),
      '#default_value' => $sid,
      '#options' => $states,
      '#description' => t('There are !total existing issues with the status of @name. Please select a new status for these issues.', array('!total' => $total, '@name' => $name)),
    );
  }
  $form['sid'] = array(
    '#type' => 'value',
    '#value' => $sid,
  );
  $form['name'] = array(
    '#type' => 'hidden',
    '#value' => $name,
  );
  return confirm_form(
    $form,
    t('Are you sure you want to delete the status option %name?', array('%name' => $name)),
    'admin/project/project-issue-status',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

function project_issue_delete_state_confirm_submit($form_id, $form_values) {
  if($form_values['new_sid'] == $form_values['sid']) {
      form_set_error('new_sid', t('Choose a new issue status for existing issues of status %name.', array('%name' => $form_values['name'])));
  } else {
    if ($form_values['new_sid']) {
      db_query('UPDATE {project_issues} SET sid = %d WHERE sid = %d', $form_values['new_sid'], $form_values['sid']);
    }
    db_query('DELETE FROM {project_issue_state} WHERE sid = %d', $form_values['sid']);
    drupal_set_message(t('Project issue status %issue deleted.', array('%issue' => $form_values['name'])));
    drupal_goto('admin/project/project-issue-status');
  }
}

function project_issue_query_result($query = NULL, $format = 'html', $show_search = true, $set_title = true) {
  global $user;
  $query = project_issue_query_parse($query);
  // $query must have a real value now, or the rest of this function fails.

  $uris = NULL;
  $projects = array(0 => t('<all>')) + project_projects_select_options($uris);
  $states = array(implode(',', array_keys(project_issue_state())) => t('<all>')) + project_issue_state();
  $priorities = array(0 => t('<all>')) + project_issue_priority();

  // Load active project
  if (count($query->projects) == 1) {
    $project = node_load($query->projects[0]);
  }

  if ($project) {
    if ($set_title) {
      drupal_set_title(t('Issues for %name', array('%name' => $project->title)));
      project_project_set_breadcrumb($project, TRUE);
    }
    if (module_exists('project_release')) {
      $releases = project_release_get_releases($project, 0);
    }
    $query->projects = $project->nid;
    $links = array();
    if (node_access('create', 'project_issue')) {
      $links[] = array(
        'title' => t('Create'),
        'href' => "node/add/project-issue/$project->uri",
        'attributes' => array('title' => t('Create a new issue for @project.', array('@project' => $project->uri))),
      );
    }
    else {
      $links[] = array(
        'title' => theme('project_issue_create_forbidden', $project->uri),
        'html'  => TRUE,
      );
    }
    $links[] = array(
      'title' => t('Statistics'),
      'href' => "project/issues/statistics/$project->uri",
      'attributes' => array('title' => t('See statistics about @project issues.', array('@project' => $project->uri))),
    );
    if ($user->uid) {
      $links[] = array(
        'title' => t('Subscribe'),
        'href' => "project/issues/subscribe-mail/$project->uri",
        'attributes' => array('title' => t('Receive email updates about @project issues.', array('@project' => $project->uri))),
      );
    }
    $links[] = array(
      'title' => t('Advanced search'),
      'href' => "project/issues/search/$project->uri",
      'attributes' => array('title' => t('Use the advanced search page to find @project issues.', array('@project' => $project->uri))),
    );
  }
  else {
    // only set a more descriptive title if we're not looking at "my
    // issues" or "my projects", since those set their own title already.
    if (!isset($_GET['q']) || !strpos($_GET['q'], '/user')) {
      drupal_set_title(t('Issues for all projects'));
    }
    $links = array();
    if (node_access('create', 'project_issue')) {
      $links[] = array(
        'title' => t('Create'),
        'href' => "node/add/project-issue",
        'attributes' => array('title' => t('Create a new issue.')),
      );
    }
    else {
      $links[] = array(
        'title' => theme('project_issue_create_forbidden', $project->uri),
        'html'  => TRUE,
      );
    }
    $links[] = array(
      'title' => t('Statistics'),
      'href' => "project/issues/statistics",
      'attributes' => array('title' => t('See statistics about issues.')),
    );
    if ($user->uid) {
      $links[] = array(
        'title' => t('Subscribe'),
        'href' => "project/issues/subscribe-mail",
        'attributes' => array('title' => t('Receive email updates about issues.')),
      );
    }
    $links[] = array(
      'title' => t('Advanced search'),
      'href' => "project/issues/search/",
      'attributes' => array('title' => t('Use the advanced search page for finding issues.')),
    );
  }

  $issues_per_page = variable_get('project_issues_per_page', PROJECT_ISSUES_PER_PAGE);

  $header = array();
  if (!$project->nid) {
    $header[] = array('data' => t('Project'), 'field' => 'p.pid');
  }
  $header[] = array('data' => t('Summary'), 'field' => 'n.title');
  $header[] = array('data' => t('Status'), 'field' => 'p.sid');
  $header[] = array('data' => t('Priority'), 'field' => 'p.priority');
  $header[] = array('data' => t('Category'), 'field' => 'p.category');
  if (!empty($releases)) {
    $header[] = array('data' => t('Version'), 'field' => 'p.rid');
  }
  $header[] = array('data' => t('Last updated'), 'field' => 'n.changed', 'sort' => 'desc');
  $header[] = array('data' => t('Assigned to'), 'field' => 'u.name');

  $sql = project_issue_query_sql($query);
  $result = pager_query($sql['sql'] . tablesort_sql($header), $issues_per_page, 0, $sql['count']);

  $search = '';
  if ($show_search) {
    // Action links:
    $search .= theme('links', $links);
    $search .= drupal_get_form('project_issue_query_result_quick_search', $query, $projects, $states, $priorities);
  }

  $rows = array();

  // For the rest of this function, we need $projects to be a flat
  // array to map pids to project names. If we're using the project
  // taxonomy, it will really be an array of arrays, where each
  // project type is itself an array. So, we flatten the array once
  // and then can use it safely in the rest of this method.
  if (project_use_taxonomy()) {
    $flat_projects = array();
    foreach ($projects as $tmp) {
      if (is_array($tmp)) {
        $flat_projects += $tmp;
      }
    }
    $projects = $flat_projects;
  }

  $rss = '';
  $link = '';
  if ($format == 'rss') {
    project_issue_query_rss($result, $project);
  }
  elseif (db_num_rows($result)) {
    while ($node = db_fetch_object($result)) {
      $node = node_load($node->nid);
      $row = array();
      $class = "state-$node->sid";
      if (!$project->nid) {
        $row[] = l($projects[$node->pid], "project/issues/$node->pid");
      }
      if (strlen($node->title) > 50) {
        $title = l(drupal_substr($node->title, 0, 50), "node/$node->nid", array('title' => $node->title));
      }
      else {
        $title = l($node->title, "node/$node->nid");
      }
      $row[] = $title . theme('mark', node_mark($node->nid, $node->changed));
      $row[] = $states[$node->sid];
      $row[] = $priorities[$node->priority];
      $row[] = project_issue_category($node->category, 0);
      if (count($releases)) {
        $row[] = $releases[$node->rid];
      }
      $row[] = array('data' => format_interval(time() - $node->changed, 2), 'align' => 'right');
      $row[] = ($node->assigned) ? theme('username', user_load(array('uid' => $node->assigned))) : '';

      $row = array('data' => $row, 'class' => $class);

      $rows[] = $row;
    }
    $query = project_issue_query_pager($query);

    $rss = theme('feed_icon', url('project/issues/rss', project_issue_query_url($query)));
    $link = l(t('#'), 'project/issues', array('title' => t('Permalink to search query.')), project_issue_query_url($query));
    if ($pager = theme('pager', NULL, $issues_per_page, 0)) {
      $rows[] = array(array('data' => $pager, 'colspan' => count($header)));
    }
  }
  else {
    $rows[] = array(array('data' => t('No issues found.'), 'colspan'=> 8));
  }

  $output = '<div class="project-issue">';
  $output .= '<div class="quick-search">';
  $output .= $search;

  $output .= '</div>';
  $output .= theme('table', $header, $rows);
  $output .= "$rss $link";

  $output .= '</div>';
  return $output;
}

/**
 * Form builder for the quick-search form when filtering issues on the
 * issue query result pages.
 */
function project_issue_query_result_quick_search($query, $projects, $states, $priorities) {
  $categories = array(0 => t('<all>')) + project_issue_category();

  // Convert array fields to single select form items.
  $fields = array('projects', 'states', 'priorities', 'categories', 'users');
  foreach ($fields as $field) {
    if (!isset($query->$field)) {
      $query->$field = '';
    }
    elseif (is_array($query->$field)) {
      $option = array();

      // $query is untrusted, user submitted data
      foreach ($query->$field as $key => $value) {
        if (isset(${$field}[$value])) {
          $option[] = ${$field}[$value];
        }
        else {
          unset($query->{$field}[$key]);
        }
      }
      $query->$field = implode(',', $query->$field);

      // special case for 'states' so we don't lose <all>
      if (!isset(${$field}[$query->$field])) {
        ${$field}[$query->$field] = implode(',', $option);
      }
    }
  }

  // special case for states - we always want the defaults
  $default_states = project_issue_default_states();
  if ($query->states != implode(',', $default_states)) {
    foreach ($default_states as $state) {
      $options[] = $state;
      $values[]  = $states[$state];
    }
    $options = implode(',', $options);
    $states[$options] =  implode(',', $values);
  }

  $form['projects'] = array(
    '#type'=> 'select',
    '#title' => t('Project'),
    '#default_value' => $query->projects,
    '#options' => $projects,
  );
  $form['states'] = array(
    '#type'=> 'select',
    '#title' => t('Status'),
    '#default_value' => $query->states,
    '#options' => $states,
  );
  $form['categories'] = array(
    '#type'=> 'select',
    '#title' => t('Category'),
    '#default_value' => $query->categories,
    '#options' => $categories,
  );
  $form['priorities'] = array(
    '#type'=> 'select',
    '#title' => t('Priority'),
    '#default_value' => $query->priorities,
    '#options' => $priorities,
  );
  $form['users'] = array(
    '#type'=> 'hidden',
    '#value' => $query->users,
  );
  $form['js'] = array(
    '#type'=> 'hidden',
    '#value' => '0',
  );
  $form['submit'] = array(
    '#type' => 'button',
    '#value' => t('Search'),
  );
  return $form;
}

function theme_project_issue_query_result_quick_search($form) {
  $rows[] = array(
    drupal_render($form['projects']),
    drupal_render($form['states']),
    drupal_render($form['categories']),
    drupal_render($form['priorities']),
    drupal_render($form['users']) . drupal_render($form['submit']),
  );
  $output = theme('table', array(), $rows);
  $output .= drupal_render($form);
  return $output;
}

function project_issue_query_url($query = 0) {
  static $url = NULL;

  if (is_array($query) && $url === NULL) {
    foreach ($query as $key => $value) {
      $url[] = "$key=$value";
    }
    $url = implode('&', $url);
  }
  return $url;
}

/**
 * Parses $_POST or $_GET and creates the appropriate query
 * object based on whatever choices a user has made to filter the
 * issue queue. This function *MUST* return a valid $query object,
 * since the rest of project_issue_query_result() assumes that $query
 * exists and contains some data. If the query was otherwise going to
 * be empty, this function will automatically add the default set of
 * issue states (active, fixed, patch needs work, etc).
 *
 * @param $query
 *   The existing query object we're adding to. (This should probably
 *   be a reference, instead of returning the object again).
 *
 * @return
 *   An object that describes what kind of query we're doing.
 *
 * @see project_issue_query_result()
 */
function project_issue_query_parse($query = NULL) {
  $fields = array('projects', 'text', 'attachment', 'summary', 'comment', 'categories', 'components', 'versions', 'states', 'priorities', 'users', 'assigned', 'submitted', 'participated');
  if ($_SERVER['REQUEST_METHOD'] == 'POST' && is_array($_POST)) {
    foreach ($_POST as $key => $value) {
      if (!empty($value) && in_array($key, $fields)) {
        $query->$key = !is_array($value) ? explode(',', $value) : $value;
      }
    }
  }
  else {
    foreach ($_GET as $key => $value) {
      if (!empty($value) && in_array($key, $fields)) {
        $query->$key = explode(',', $value);
      }
    }
  }

  if (empty($query->states)) {
    $query->states = project_issue_default_states();
  }
  if ($query->states[0] == 'all') {
    $query->states = array_keys(project_issue_state());
  }
  if (!empty($_REQUEST['participated']) && empty($query->participated)) {
    $query->participated = (int)$_REQUEST['participated'];
  }

  return $query;
}

function project_issue_query_sql_field($field, $values, $like = 0, $operator = ' OR ', $callback = 0) {
  $sql = array();
  if (!is_array($values)) {
    $values = array($values);
  }
  foreach ($values as $value) {
    $value = db_escape_string($value);
    if ($callback) {
      $value = $callback($value);
    }
    $sql[] = ($like && $field != 'p.pid') ? "$field LIKE '". str_replace('%', '%%', "%$value%") ."'" : "$field = '$value'";
  }
  if ($sql) {
    return '('. implode($operator, $sql) .')';
  }
}

function project_issue_query_pager($query) {
  $get = array();
  if (count($query)) {
    foreach ($query as $key => $value) {
      $get[$key] = (is_array($value)) ? implode(',', $value) : $value;
    }
  }
  return $get;
}

function project_issue_query_sql($query) {
  $comments = 0;
  foreach ($query as $key => $value) {
    switch ($key) {
      case 'projects':
        $sql[] = project_issue_query_sql_field('p.pid', $value, 1);
        break;
      case 'text':
        $comments = 1;
        $sql[] = '('. project_issue_query_sql_field('n.title', $value, 1) .' OR '. project_issue_query_sql_field('r.body', $value, 1) .' OR '. project_issue_query_sql_field('c.body', $value, 1) .')';
        break;
      case 'attachment':
        $comments = 1;
        $sql[] = "(p.file_path <> '' OR c.file_path <> '')";
        break;
      case 'summary':
        $sql[] = '('. project_issue_query_sql_field('n.title', $value, 1) .' OR '. project_issue_query_sql_field('r.body', $value, 1) .')';
        break;
      case 'comment':
        $comments = 1;
        $sql[] = project_issue_query_sql_field('c.body', $value, 1);
        break;
      case 'categories':
        $sql[] = project_issue_query_sql_field('p.category', $value);
        break;
      case 'components':
        $sql[] = project_issue_query_sql_field('p.component', $value);
        break;
      case 'versions':
        $sql[] = project_issue_query_sql_field('p.rid', $value);
        break;
      case 'states':
        $sql[] = project_issue_query_sql_field('p.sid', $value);
        break;
      case 'priorities':
        $sql[] = project_issue_query_sql_field('p.priority', $value);
        break;
      case 'users':
        $_sql = array(
          project_issue_query_sql_field('n.uid', $value, 0, ' OR ', 'project_issue_query_user'),
          project_issue_query_sql_field('p.assigned', $value, 0, ' OR ', 'project_issue_query_user')
        );
        $sql[] = '('. implode(' OR ', $_sql) .')';
        break;
      case 'participated':
        $comments = 1;
        $_sql = array(
          project_issue_query_sql_field('n.uid', $value, 0, ' OR ', 'project_issue_query_user'),
          project_issue_query_sql_field('p.assigned', $value, 0, ' OR ', 'project_issue_query_user'),
          project_issue_query_sql_field('c.uid', $value, 0, ' OR ', 'project_issue_query_user')
        );
        $sql[] = '('. implode(' OR ', $_sql) .')';
        break;
      case 'assigned':
        $sql[] = project_issue_query_sql_field('p.assigned', $value, 0, ' OR ', 'project_issue_query_user');
        break;
      case 'submitted':
        $sql[] = project_issue_query_sql_field('n.uid', $value, 0, ' OR ', 'project_issue_query_user');
        break;
    }
  }

  if (!$comments) {
    return array(
      'sql' => db_rewrite_sql('SELECT n.nid FROM {node} n INNER JOIN {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')'),
      'count' => db_rewrite_sql('SELECT COUNT(n.nid) FROM {node} n INNER JOIN {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')')
    );
  }
  else {
    return array(
      'sql' => db_rewrite_sql('SELECT DISTINCT(n.nid), n.changed FROM {node} n INNER JOIN {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid LEFT JOIN {project_comments} c ON c.nid = p.nid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')'),
      'count' => db_rewrite_sql('SELECT COUNT(DISTINCT(n.nid)) FROM {node} n INNER JOIN  {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid LEFT JOIN {project_comments} c ON c.nid = p.nid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')')
    );
  }
}

function project_issue_query_user($value) {
  if (is_numeric($value)) {
    return $value;
  }
  else {
    $uid = db_result(db_query("SELECT uid FROM {users} WHERE name = '%s'", $value));
    if (!$uid) {
      drupal_set_message(t("Username '@user' not found.", array('@user' => $value)), 'error');
    }
    return $uid;
  }
}

function project_issue_query_rss($result, $project) {
  if ($project) {
    $channel['title'] = variable_get('site_name', 'drupal') .' - '. t('issues for !name', array('!name' => $project->title));
  }
  else {
    $channel['title'] = variable_get('site_name', 'drupal') .' - '. t('issues');
  }
  $channel['link'] = url('project/issues', NULL, NULL, TRUE); // not 100% correct

  node_feed($result, $channel);
}

function project_issue_tracker_comments($node) {
  return db_query(db_rewrite_sql("SELECT c.nid, c.cid, c.uid, c.created AS timestamp, '%s' AS subject, u.name FROM {project_comments} c INNER JOIN {users} u ON c.uid = u.uid WHERE c.nid = %d ORDER BY c.cid DESC", 'c'), $node->title, $node->nid);
}

function theme_project_issue_follow_up_forbidden($nid) {
  global $user;
  if ($user->uid) {
    return '';
  }

  // We cannot use drupal_get_destination() because these links sometimes appear on /node and taxo listing pages.
  $destination = "destination=". drupal_urlencode("project/comments/add/$nid");

  if (variable_get('user_register', 1)) {
    return t('<a href="@login">Login</a> or <a href="@register">register</a> to follow up', array('@login' => url('user/login', $destination), '@register' => url('user/register', $destination)));
  }
  else {
    return t('<a href="@login">Login</a> to follow up', array('@login' => url('user/login', $destination)));
  }
}

function theme_project_issue_create_forbidden($uri) {
  global $user;
  if ($user->uid) {
    return '';
  }

  // We cannot use drupal_get_destination() because these links sometimes appear on /node and taxo listing pages.
  $destination = "destination=". drupal_urlencode("node/add/project-issue/$uri");

  if (variable_get('user_register', 1)) {
    return t('<a href="@login">Login</a> or <a href="@register">register</a> to create an issue', array('@login' => url('user/login', $destination), '@register' => url('user/register', $destination)));
  }
  else {
    return t('<a href="@login">Login</a> to create an issue', array('@login' => url('user/login', $destination)));
  }
}
