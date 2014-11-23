<?php
// $Id: issue.inc,v 1.304.2.17 2009/01/07 20:48:32 dww Exp $

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
 * JS callback method to return updated elements on the issue form.
 * This function is called when someone changes the "Project" selector.
 * See project_issue.js.
 *
 * @param $pid
 *   The nid of the project we're updating to.
 * @param $issue_nid
 *   The issue nid.
 * @param $cid
 *   The component from the previously selected project.
 * @param $rid
 *   The version from the previously selected project.
 * @param $assigned_uid
 *   The uid of the assigned user from the previously selected project.
 */
function project_issue_update_project($pid, $issue_nid, $cid = NULL, $rid = NULL, $assigned_uid = NULL) {
  // Prevent a malicious user from snooping this callback directly.
  $error = array('error' => t('An error was detected. Please try again.'));
  $node = node_load(array('nid' => $pid, 'type' => 'project_project'));
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
      // Element is tree'd here to match the original form layout.
      $form['project_info']['#tree'] = TRUE;
      $form['project_info']['rid'] = array(
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

  // Assigned.
  if (is_numeric($issue_nid)) {
    $issue = node_load(array('nid' => $issue_nid, 'type' => 'project_issue'));
  }
  else {
    // @TODO:  This case should not ever be hit until
    // http://drupal.org/node/197281 lands.
    $issue = new stdClass;
  }
  $issue->pid = $pid;
  $return['assigned'] = '';
  $assigned_choices = project_issue_assigned_choices($issue);

  $form = array();
  $form['issue_info']['assigned'] = array(
    '#type' => 'select',
    '#title' => t('Assigned'),
    '#default_value' => $assigned_uid,
    '#options' => $assigned_choices,
  );

  // Build the HTML output for the assigned select.
  $form = form_builder('assigned', $form);
  $return['assigned'] .= drupal_render($form);

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
  // Element is tree'd here to match the original form layout.
  $form['project_info']['#tree'] = TRUE;
  $form['project_info']['component'] = array(
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
  $states = project_issue_state();
  if ($project->nid) {
    $filter = sprintf(' AND p.pid = %d ', (int)$project->nid);
    project_project_set_breadcrumb($project, TRUE);
  }

  $output = '<div class="project-issue">';

  // Issue lifetime.
  $header = array(t('Category'), t('Overall'), t('Last month'));
  $rows = array();
  $duration = time() - 30 * 24 * 60 * 60;
  $result = db_query(db_rewrite_sql('SELECT p.category, SUM(n.changed - n.created) / COUNT(p.category) AS duration FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND p.sid > 1 GROUP BY p.category'), $filter);
  while ($stat = db_fetch_object($result)) {
    $rows[$stat->category][0] = project_issue_category($stat->category);
    $rows[$stat->category][1] = array('data' => format_interval($stat->duration, 2), 'class' => 'numeric');
    $rows[$stat->category][2] = array('data' => t('N/A'), 'class' => 'numeric');
  }

  $result = db_query(db_rewrite_sql('SELECT p.category, SUM(n.changed - n.created) / COUNT(p.category) AS duration FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND p.sid > 1 AND n.created > %d GROUP BY p.category'), $filter, $duration);
  while ($stat = db_fetch_object($result)) {
    if ($stat->duration > 0) {
      $rows[$stat->category][2] = array('data' => format_interval($stat->duration, 2), 'class' => 'numeric');
    }
  }
  $output .= '<h2>'. t('Average lifetime') .'</h2>';
  $output .= theme('table', $header, $rows);

  $header = array(
    array('data' => t('Status')),
    array('data' => t('Overall'), 'class' => 'project-issue-numeric'),
    array('data' => '%', 'class' => 'project-issue-numeric'),
    array('data' => t('Last month'), 'class' => 'project-issue-numeric'),
  );
  $rows = array();
  // Activity overall.
  $total = db_result(db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s', 'p'), $filter));
  $result = db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total, p.sid FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s GROUP BY p.sid', 'p'), $filter);
  while ($stat = db_fetch_object($result)) {
    $rows[$stat->sid][0] = check_plain(project_issue_state($stat->sid));
    $rows[$stat->sid][1] = array('data' => $stat->total, 'class' => 'project-issue-numeric');
    $rows[$stat->sid][2] = array('data' => number_format($stat->total / $total * 100) .'%', 'class' => 'project-issue-numeric-light');
    $rows[$stat->sid][3] = array('data' => '0', 'class' => 'project-issue-numeric');
  }
  // Activity this month.
  $result = db_query(db_rewrite_sql('SELECT COUNT(p.nid) AS total, p.sid FROM {project_issues} p INNER JOIN {node} n ON n.nid = p.nid WHERE n.status = 1%s AND n.changed > %d GROUP BY p.sid', 'p'), $filter, $duration);
  while ($stat = db_fetch_object($result)) {
    $rows[$stat->sid][3] = array('data' => $stat->total, 'class' => 'project-issue-numeric');
  }
  $output .= '<h2>'. t('Issue activity') .'</h2>';
  $output .= theme('table', $header, $rows);

  // Project overview.
  if (!$project->nid) {
    // Even though we don't use the tablesorting logic in the query itself,
    // we include it anyways because we're going to leverage the $_GET arguments
    // to build our own tablesorting mechanism.
    $header = array();
    $header['project'] = array('data' => t('Project'), 'field' => 'title');
    foreach ($states as $key => $value) {
      $header[$key] = array('data' => check_plain($value), 'field' => $key);
    }
    $header['total'] = array('data' => t('Total'), 'field' => 'total');
    // Force sorting arrow to appear on active first.
    $header[1]['sort'] = 'desc';
    $args = array();

    // Since we're pulling the sid to sort by here individually in the first query
    // below, we can bastardize the tablesorting logic to get tablesorting.
    $where = ' AND p.sid = %d';
    $column = 'total';
    if (isset($_GET['order'])) {
      switch ($_GET['order']) {
        case 'Project':
          $where = '';
          $column = 'title';
          break;
        case 'Total':
          $where = '';
          break;
        default:
          if ($state = array_search($_GET['order'], $states)) {
            $args[] = $state;
          }
          else {
            $args[] = 1;
          }
          break;
      }
    }
    else {
      $args[] = 1;
    }
    $sort = (isset($_GET['sort']) && $_GET['sort'] == 'desc') || !isset($_GET['sort']) ? 'DESC' : 'ASC';

    $rows = array();
    $projects = pager_query(db_rewrite_sql("SELECT pn.nid, pn.title, COUNT(n.nid) AS total FROM {node} n INNER JOIN {project_issues} p ON n.nid = p.nid INNER JOIN {node} pn ON p.pid = pn.nid WHERE n.status = 1 AND pn.status = 1$where GROUP BY pn.nid, pn.title ORDER BY $column $sort"), 15, 0, db_rewrite_sql("SELECT COUNT(DISTINCT(n.nid)) FROM {node} n INNER JOIN {project_issues} p ON n.nid = p.pid INNER JOIN {node} pin ON p.nid = pin.nid WHERE n.status = 1 AND pin.status = 1$where"), $args);
    $orig = array('project' => array('data' => 0));
    foreach ($states as $key => $value) {
      $orig[$key] = array('data' => '', 'class' => 'project-issue-numeric');
    }
    $orig['total'] = array('data' => '', 'class' => 'project-issue-numeric');

    while ($project = db_fetch_object($projects)) {
      $rows[$project->nid] = $orig;
      $rows[$project->nid]['project']['data'] = l($project->title, "node/$project->nid");
      $stats = db_query("SELECT sid, COUNT(nid) as total FROM {project_issues} WHERE pid = %d GROUP BY sid", $project->nid);
      while ($stat = db_fetch_object($stats)) {
        $rows[$project->nid]['total']['data'] += $stat->total;
        $rows[$project->nid][$stat->sid]['data'] = $stat->total;
      }
    }
    $output .= '<h2>'. t('Project overview') .'</h2>';
    $output .= '<div class="project-issue-statistics-overview-table">';
    $output .= theme('table', $header, $rows);

    if ($pager = theme('pager', 15, 0)) {
      $output .= $pager;
    }
    $output .= '</div>';
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

function project_issue_pick_project_page() {
  $types = node_get_types();
  drupal_set_title(t('Submit @name', array('@name' => $types['project_issue']->name)));
  return drupal_get_form('project_issue_pick_project_form');
}

/**
 * Form builder for a simple form to select a project when creating a new
 * issue (as the first "page", but this is not really a multi-page form).
 */
function project_issue_pick_project_form() {
  $form = array();

  // Fetch a list of all projects.
  $uris = NULL;
  $projects = array(t('<none>')) + project_projects_select_options($uris);
  if (count($projects) == 1) {
    drupal_set_message(t('You do not have access to any projects.'), 'error');
  }

  $form['pid'] = array(
    '#type' => 'select',
    '#title' => t('Project'),
    '#options' => $projects,
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}

function project_issue_pick_project_form_validate($form_id, $form_values) {
  if (empty($form_values['pid'])) {
    form_set_error('pid', t('You must select a project.'));
  }
  $node = node_load($form_values['pid']);
  if (empty($node) || $node->type != 'project_project') {
    form_set_error('pid', t('Invalid project selected.'));
  }
}

function project_issue_pick_project_form_submit($form_id, $form_values) {
  $project = node_load($form_values['pid']);
  return 'node/add/project-issue/'. $project->uri;
}

/**
 * Implementation of hook_form().
 *
 * Create the project issue node form.
 *
 * @param $node
 *   The project issue node object.
 * @param $include_metadata_fields
 *   If set, metadata fields (eg. status, assigned, title) will
 *   be included in the form regardless of whether $node->nid is set.
 *   Otherwise, metadata fields will only be included in the form
 *   if $node->nid is empty.
 */
function project_issue_form($node, $include_metadata_fields = FALSE) {
  global $user;

  // If this function is being called as a result of CCK building the form
  // in content_admin_field_overview_form(), just return an empty array, since
  // CCK only grabs the top level fields and this form has none of those anyway.
  if (!empty($node->cck_dummy_node_form)) {
    return array();
  }

  if (arg(0) == 'node' && arg(1) == 'add') {
    $breadcrumb = array();
    $breadcrumb[] = l(t('Home'), NULL);
    $breadcrumb[] = l(t('Create content'), 'node/add');
    drupal_set_breadcrumb($breadcrumb);
  }

  $default_state = variable_get('project_issue_default_state', 1);

  $form['#prefix'] = '<div class="project-issue">';
  $form['#suffix'] = '</div>';

  // Fetch a list of all projects to test for access.
  $uris = NULL;
  $projects = array(t('<none>')) + project_projects_select_options($uris);
  if (count($projects) == 1) {
    drupal_set_message(t('You do not have access to any projects.'), 'error');
    drupal_goto('node/add/project-issue');
    return;
  }

  if (empty($node->pid)) {
    $pid = arg(3);
    if (!empty($pid)) {
      if (is_numeric($pid)) {
        $node->pid = db_result(db_query(db_rewrite_sql('SELECT p.nid FROM {project_projects} p WHERE p.nid = %d', 'p'), $pid), 0);
      }
      else {
        $node->pid = db_result(db_query(db_rewrite_sql("SELECT p.nid FROM {project_projects} p WHERE p.uri = '%s'", 'p'), $pid), 0);
      }
    }
  }
  $pid = $node->pid;

  if (empty($pid)) {
    drupal_set_message(t('Invalid project selected.'), 'error');
    drupal_goto('node/add/project-issue');
    return;
  }

  // If this issue has already been created and is just being
  // edited, we want to prevent any metadata changes.  However, allow
  // the $include_metadata_fields parameter to override this check.
  if ($include_metadata_fields) {
    $allow_metadata_changes = TRUE;
  }
  else {
    $allow_metadata_changes = empty($node->nid);
  }

  // Load Javascript (unless the issue is being edited).
  if ($allow_metadata_changes) {
    drupal_add_js(drupal_get_path('module', 'project_issue') .'/project_issue.js');
    drupal_add_js(
      array(
        'projectUrl' => url('project/issues/update_project'),
        'issueNid' => $node->nid,
      ), 'setting');
  }

  // Load the project and initialize some support arrays.
  $project = node_load(array('nid' => $pid, 'type' => 'project_project'));

  if ($allow_metadata_changes) {
    if (module_exists('project_release') &&
        $releases = project_release_get_releases($project, 0, 'version', 'all', array($node->rid))) {
      $releases = array(t('<none>')) + $releases;
    }
    // Remove releases marked as invalid release nodes for user selection.
    foreach (variable_get('project_issue_invalid_releases', array()) as $rid) {
      unset($releases[$rid]);
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
    $assigned = project_issue_assigned_choices($node);
  }

  // Display the site-wide and/or per-project help text.
  $site_help = trim(variable_get('project_issue_site_help', ''));
  if (!empty($site_help)) {
    $form['project_help']['site'] = array(
      '#prefix' => '<div class="messages status site">',
      '#value' => filter_xss($site_help),
      '#suffix' => '</div>',
    );
  }
  $project_help = trim($project->help);
  if (!empty($project_help)) {
    $form['project_help']['project'] = array(
      '#prefix' => '<div class="messages status project">',
      '#value' => filter_xss($project_help),
      '#suffix' => '</div>',
    );
  }

  if ($allow_metadata_changes) {
    $form['project_info'] = array(
      '#type' => 'fieldset',
      '#title' => t('Project information'),
      '#prefix' => '<div class="inline-options">',
      '#suffix' => '</div>',
    );
    $form['project_info']['project_display'] = array(
      '#type' => 'item',
      '#title' => t('Project'),
      '#value' => check_plain($project->title),
    );
    $form['project_info']['pid'] = array(
      '#type' => 'value',
      '#value' => $node->pid,
    );
    if ($releases) {
      $form['project_info']['rid'] = array(
        '#type' => 'select',
        '#title' => t('Version'),
        '#default_value' => $node->rid,
        '#options' => $releases,
        '#required' => TRUE,
      );
    }
    $form['project_info']['component'] = array(
      '#type' => 'select',
      '#title' => t('Component'),
      '#default_value' => $node->component,
      '#options' => $components,
      '#required' => TRUE,
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
      '#required' => TRUE,
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
        '#value' => check_plain(project_issue_state($default_state)),
      );
    }
  }

  $form['issue_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Issue details'),
    '#prefix' => '</div><div class="standard">',
  );

  if ($allow_metadata_changes) {
    $form['issue_details']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#default_value' => $node->title,
      '#size' => 60,
      '#maxlength' => 128,
      '#required' => TRUE,
    );
  }

  $form['issue_details']['body'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $node->body,
    '#rows' => 10,
    '#required' => TRUE,
  );
  $form['issue_details']['format'] = filter_form($node->format);

  $directory = file_create_path(variable_get('project_directory_issues', 'issues'));
  if (!file_check_directory($directory, 0)) {
    $msg = t('File attachments are disabled. The issue directory has not been properly configured.');
    if (user_access('administer site configuration')) {
      $msg .= ' '. t('Please visit the !admin-project-issue-settings page.', array('!admin-project-issue-settings' => l(t('Project issue settings'), 'admin/project/project-issue-settings')));
    }
    else {
      $msg .= ' '. t('Please contact the site administrator.');
    }
    drupal_set_message($msg, 'error');
  }
  return $form;
}

/**
 * Build an array of users to whom an issue may be assigned.
 *
 * @param $issue
 *   The issue object.
 * @return
 *   A keyed array in the form uid => name containing users
 *   to whom an issue may be assigned.
 */
function project_issue_assigned_choices($issue) {
  // Setup the array of choices for who the issue is assigned to.
  $assigned = array();
  foreach (module_implements('project_issue_assignees') as $module) {
    $function = "{$module}_project_issue_assignees";
    $function($assigned, $issue);
  }
  natcasesort($assigned);
  $assigned = array(0 => empty($issue->assigned) ? t('Unassigned') : t('Unassign')) + $assigned;
  return $assigned;
}

/**
 * Implmentation of hook_validate().
 *
 * Ensures that the issue node form has valid values for all required fields.
 * We use hook_validate() here instead of a #validate handler or even defining
 * project_issue_node_form_validate() since if we did, node_form_validate()
 * itself would not be invoked, which would lead to all kinds of problems,
 * including hook_nodeapi('validate') never being invoked.
 */
function project_issue_validate(&$node) {
  // If $node->nid is set, that means that the node was being
  // edited and not created.  If that's the case, the user was
  // not presented with any of the metadata fields, so there's no
  // need to validate them here.
  if (empty($node->nid)) {
    if (empty($node->pid)) {
      form_set_error('pid', t('You have to specify a valid project.'));
    }
    elseif ($project = node_load($node->pid)) {
      if (module_exists('project_release') &&
          $releases = project_release_get_releases($project, 0)) {
        if (empty($node->rid)) {
          form_set_error('rid', t('You have to specify a valid version.'));
        }
      }
      if (isset($node->component) && !in_array($node->component, $project->components)) {
        $node->component = 0;
      }
      if (empty($node->component)) {
        form_set_error('component', t('You have to specify a valid component.'));
      }
      if (empty($node->category)) {
        form_set_error('category', t('You have to specify a valid category.'));
      }
    }
  }
}

function project_issue_view($node, $teaser = false, $page = false) {
  $node = node_prepare($node, $teaser);

  if (!$teaser && ($page || project_issue_is_comment_reply())) {

    $node->content['#prefix'] = '<div class="project-issue">';
    $node->content['#suffix'] = '</div>';

    $project = node_load(array('nid' => $node->pid, 'type' => 'project_project'));
    $release->nid = $node->rid;
    if (module_exists('project_release')) {
      $release = project_release_load($release);
    }
    $assigned = ($node->assigned && ($account = user_load(array('uid' => $node->assigned))) ? $account->name : t('Unassigned'));

    $current_data = array();
    $current_data['pid'] = array(
      'label' => t('Project'),
      'current' => $project->title,
    );
    if ($release->version) {
      $current_data['rid'] = array(
        'label' => t('Version'),
        'current' => $release->version,
      );
    }
    $current_data['component'] = array(
      'label' => t('Component'),
      'current' => $node->component,
    );
    $current_data['category'] = array(
      'label' => t('Category'),
      'current' => project_issue_category($node->category, 0),
    );
    $current_data['priority'] = array(
      'label' => t('Priority'),
      'current' => project_issue_priority($node->priority),
    );
    $current_data['assigned'] = array(
      'label' => t('Assigned'),
      'current' => $assigned,
    );
    $current_data['sid'] = array(
      'label' => t('Status'),
      'current' => project_issue_state($node->sid),
    );

    // Allow modules to alter the metadata displayed in the table on the actual
    // issue node itself (at the very top of the issue). Modules should accept
    // the $current_data parameter by reference and add additional
    // elements for additional lines in the table.
    //
    // Modules implementing this hook should take the following parameters:
    // @param $view
    //  A string representing the metadata view being generated.  For the issue
    //  node main table, this will be 'current'.
    // @param $node
    //  The project_issue node object.
    // @param $current_data
    //  An associative array of rows in the project issue metadata table that
    //  will be displayed, with the following key/value pairs:
    //    'label' => The metadata label.
    //    'current' => The current metadata value.
    //  This parameter should be accepted by reference.
    foreach (module_implements('project_issue_metadata') as $module) {
      $function = $module .'_project_issue_metadata';
      $function('current', $node, $current_data);
    }

    $node->content['project_issue_summary'] = array(
      '#value' => theme('project_issue_summary', $current_data, project_issue_internal_links($node)),
      '#weight' => -5,
    );

    $node->content['project_issue_header'] = array(
      '#value' => '<div class="header">'. t('Description') .'</div>',
      '#weight' => -3,
    );

    project_issue_set_breadcrumb($node, $project);
  }
  return $node;
}

/**
 * Implementation of hook_project_issue_assignees().
 *
 * This hook is used to modify the list of 'Assigned' users when creating or
 * commenting on an issue.
 *
 * @param $assigned
 *  An array of key => value pairs in the format of uid => name that represents
 *  the list of users that the issue may be assigned to. This parameter
 *  is passed by reference so the hook may make changes, such as adding or
 *  removing elements.
 * @param $node
 *  The node object for the issue.
 */
function project_issue_project_issue_assignees(&$assigned, $node) {
  global $user;
  if ($user->uid) {
    if (isset($node->assigned) && $user->uid != $node->assigned) {
      // Assigned to someone else, add the currently assigned user.
      $account = user_load(array('uid' => $node->assigned));
      $assigned[$node->assigned] = $account->name;
    }
    // Always let the person replying assign it to themselves.
    $assigned[$user->uid] = $user->name;
  }

  if (user_access('assign and be assigned project issues')) {
    // All users are included if either anon or auth user has the perm.
    if (db_result(db_query("SELECT rid FROM {permission} WHERE perm LIKE '%%%s%%' AND rid IN(%d, %d)", 'assign and be assigned project issues', DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
      $result = db_query("SELECT uid, name FROM {users}");
    }
    else {
      $result = db_query("SELECT u.uid, u.name FROM {users} u INNER JOIN {users_roles} ur ON u.uid = ur.uid INNER JOIN {role} r ON ur.rid = r.rid INNER JOIN {permission} p ON p.rid = r.rid WHERE p.perm LIKE '%%%s%%'", 'assign and be assigned project issues');
    }

    while ($assignee = db_fetch_object($result)) {
      $assigned[$assignee->uid] = $assignee->name;
    }
  }
}

/**
 * Themes the metadata table and internal page links for issue nodes.
 *
 * @param $current_data
 *   An array of current issue data for the metadata table.
 * @param $summary_links
 *   An array of internal page links.
 * @return
 *   An HTML string of the summary section.
 */
function theme_project_issue_summary($current_data, $summary_links) {
  $allowed_tags = array('a', 'em', 'strong', 'div', 'span');
  // Fields that should be rendered as plain text, not filtered HTML.
  $plain_fields = array('title', 'pid', 'rid');

  $rows = array();
  foreach ($current_data as $name => $values) {
    $row = array();
    $row[] = filter_xss($values['label'], $allowed_tags) .':';
    if (in_array($name, $plain_fields)) {
      $row[] = check_plain($values['current']);
    }
    else {
      $row[] = filter_xss($values['current'], $allowed_tags);
    }
    $rows[] = $row;
  }

  $output = '<div id="project-summary-container" class="clear-block">';
  $output .= '<div id="project-issue-summary-table" class="summary">'. theme('table', array(), $rows) .'</div>';
  if (!empty($summary_links)) {
    $output .= '<div id="project-issue-summary-links">'. theme('item_list', $summary_links, t('Jump to:'), 'ul', array('class' => 'internal-links')) .'</div>';
  }
  $output .= '</div>';

  return $output;
}

/**
 * Generates internal page links for issue pages.
 *
 * @param $node
 *   The issue node.
 * @return
 *   An array of internal page links.
 */
function project_issue_internal_links($node) {
  $links = array();

  if ($node->comment != COMMENT_NODE_DISABLED) {
    // Link to the first unread, or most recent comment.
    if (comment_num_new($node->nid)) {
      // There are unread replies; link to first unread comment.
      $links[] = l(t('First unread comment'), "node/$node->nid", array(), NULL, 'new');
    }
    else {
      // No unread replies; link to most recent comment.
      $comment_cid = db_result(db_query_range("SELECT pic.cid FROM {project_issue_comments} pic INNER JOIN {node} n on pic.nid = n.nid WHERE n.status = 1 AND n.nid = %d ORDER BY pic.cid DESC", $node->nid, 0, 1));
      if ($comment_cid) {
        $links[] = l(t('Most recent comment'), "node/$node->nid", array(), NULL, "comment-$comment_cid");
      }
    }
    // Link for most recent patch.
    $file_cid = db_result(db_query_range("SELECT cu.cid FROM {comment_upload_files} cu INNER JOIN {node} n on cu.nid = n.nid WHERE n.status = 1 AND n.nid = %d ORDER BY cu.fid DESC", $node->nid, 0, 1));
    if ($file_cid) {
      $links[] = l(t('Most recent attachment'), "node/$node->nid", array(), NULL, "comment-$file_cid");
    }
  }

  // Link straight to comment form.
  if ($node->comment == COMMENT_NODE_READ_WRITE && (user_access('post comments') || user_access('post comments without approval'))) {
    // TODO: This conditional needs to be ripped out in D6.
    $comment_form_location = isset($node->comment_form_location) ? $node->comment_form_location : variable_get('comment_form_location', COMMENT_FORM_SEPARATE_PAGE);

    // Comment form isn't on the page, link to the comment reply page.
    if ($comment_form_location == COMMENT_FORM_SEPARATE_PAGE) {
      $links[] = l(t('Add new comment'), "comment/reply/$node->nid");
    }
    // Comment form is on the page, generate an internal page link for it.
    else {
      $links[] = l(t('Add new comment'), "node/$node->nid", array(), NULL, "comment-form");
    }
  }

  return $links;
}

function project_issue_load($node) {
  $project = db_fetch_object(db_query(db_rewrite_sql('SELECT pi.* FROM {project_issues} pi WHERE pi.nid = %d', 'pi'), $node->nid));

  // TODO: This need to be ripped out in D6.
  $project->comment_form_location = variable_get('project_issue_comment_form_location', NULL);

  return $project;
}

function project_issue_insert($node) {

  // Permanently store the original issue states in a serialized array. This is a bit
  // yucky, but we need them for proper handling of states workflow.  The current states
  // need to be stored in {project_issues} as well for query efficiency in issue queue
  // searches, and it seems too messy to add a bunch of new columns to the {project_issues}
  // table for the original states.
  $original_issue_data = new stdClass();
  $fields = array(
    'pid' => 0,
    'rid' => 0,
    'component' => '',
    'category' => '',
    'priority' => 0,
    'assigned' => 0,
    'sid' => 0,
    'title' => '',
  );
  foreach ($fields as $field => $default) {
    // Some of the incoming data may not have the correct default.
    if (!$node->$field) {
      $node->$field = $default;
    }
    $original_issue_data->$field = $node->$field;
  }

  db_query("INSERT INTO {project_issues} (nid, pid, category, component, priority, rid, assigned, sid, original_issue_data, last_comment_id, db_lock) VALUES (%d, %d, '%s', '%s', %d, %d, %d, %d, '%s', %d, %d)", $node->nid, $node->pid, $node->category, $node->component, $node->priority, $node->rid, $node->assigned, $node->sid, serialize($original_issue_data), 0, 0);
}

function project_issue_delete($node) {
  db_query('DELETE FROM {project_issues} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {project_issue_comments} WHERE nid = %d', $node->nid);
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
 *   NOTE: The state name returned is raw text, and should be filtered via
 *   check_plain() or filter_xss() before being printed as HTML output.
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
    $project = node_load(array('nid' => (int) $query->projects[0], 'type' => 'project_project'));
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
      $links['create'] = array(
        'title' => t('Create'),
        'href' => "node/add/project-issue/$project->uri",
        'attributes' => array('title' => t('Create a new issue for @project.', array('@project' => $project->uri))),
      );
    }
    else {
      $links['create'] = array(
        'title' => theme('project_issue_create_forbidden', $project->uri),
        'html'  => TRUE,
      );
    }
    $links['statistics'] = array(
      'title' => t('Statistics'),
      'href' => "project/issues/statistics/$project->uri",
      'attributes' => array('title' => t('See statistics about @project issues.', array('@project' => $project->uri))),
    );
    if ($user->uid) {
      $links['subscribe'] = array(
        'title' => t('Subscribe'),
        'href' => "project/issues/subscribe-mail/$project->uri",
        'attributes' => array('title' => t('Receive email updates about @project issues.', array('@project' => $project->uri))),
      );
    }
    $links['search'] = array(
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
      $links['create'] = array(
        'title' => t('Create'),
        'href' => "node/add/project-issue",
        'attributes' => array('title' => t('Create a new issue.')),
      );
    }
    else {
      $links['create'] = array(
        'title' => theme('project_issue_create_forbidden', $project->uri),
        'html'  => TRUE,
      );
    }
    $links['statistics'] = array(
      'title' => t('Statistics'),
      'href' => "project/issues/statistics",
      'attributes' => array('title' => t('See statistics about issues.')),
    );
    if ($user->uid) {
      $links['subscribe'] = array(
        'title' => t('Subscribe'),
        'href' => "project/issues/subscribe-mail",
        'attributes' => array('title' => t('Receive email updates about issues.')),
      );
    }
    $links['search'] = array(
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
    $search .= theme('project_issue_query_result_links', $links);
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
      $row[] = check_plain($states[$node->sid]);
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

    drupal_add_feed(url('project/issues/rss', project_issue_query_url($query)), t('Issues'));
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
  $output .= $link;

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

/**
 * Return the HTML to use for the links at the top of issue query pages.
 *
 * @param $links
 *   Array of links, indexed by the action ('create', 'search', etc).
 *
 * @see project_issue_query_result()
 * @see theme_links()
 */
function theme_project_issue_query_result_links($links) {
  return theme('links', $links);
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
        foreach (preg_split('/\s+/', $value[0]) as $word) {
          $sql[] = '('. project_issue_query_sql_field('n.title', $word, 1) .' OR '. project_issue_query_sql_field('r.body', $word, 1) .' OR '. project_issue_query_sql_field('c.comment', $word, 1) .')';
        }
        break;
      case 'attachment':
        $comments = 1;
        $sql[] = "(f.filepath <> '' OR cu.filepath <> '')";
        break;
      case 'summary':
        $sql[] = '('. project_issue_query_sql_field('n.title', $value, 1) .' OR '. project_issue_query_sql_field('r.body', $value, 1) .')';
        break;
      case 'comment':
        $comments = 1;
        $sql[] = project_issue_query_sql_field('c.comment', $value, 1);
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
      'sql' => db_rewrite_sql('SELECT DISTINCT(n.nid), n.changed FROM {node} n INNER JOIN {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid LEFT JOIN {comments} c ON c.nid = p.nid LEFT JOIN {comment_upload_files} cu ON p.nid = cu.nid LEFT JOIN {files} f ON n.nid = f.nid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')'),
      'count' => db_rewrite_sql('SELECT COUNT(DISTINCT(n.nid)) FROM {node} n INNER JOIN  {project_issues} p ON p.nid = n.nid INNER JOIN {node_revisions} r ON r.vid = n.vid INNER JOIN {users} u ON p.assigned = u.uid LEFT JOIN {comments} c ON c.nid = p.nid LEFT JOIN {comment_upload_files} cu ON p.nid = cu.nid LEFT JOIN {files} f ON n.nid = f.nid WHERE n.status = 1 AND ('. implode(') AND (', $sql) .')')
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

/**
 * Provide an array of project issue summary fields.
 * @param $context
 *   The context in which the fields will be displayed.
 *     'web' for project_issue node and comment summary tables.
 *     'email' for project_issue notification e-mail messages.
 */
function project_issue_field_labels($context) {

  if ($context == 'web') {
    $labels = array('title' => t('Title'));
  }
  else {
    $labels = array();
  }

  $labels += array(
    'pid' => t('Project'),
    'rid' => t('Version'),
    'component' => t('Component'),
    'category' => t('Category'),
    'priority' => t('Priority'),
    'assigned' => t('Assigned to'),
    'sid' => t('Status'),
  );

  if ($context == 'email') {
    $labels +=  array(
      'name' => t('Reported by'),
      'updator' => t('Updated by'),
    );
  }
  return $labels;
}

/**
 * Provide the text displayed to a user for a specific metadata field.
 *
 * @param $field
 *   Name of the field.
 * @param $value
 *   Value of the field.
 *
 * @return
 *   Text to display to user. NOTE: This is unfiltered text! If this output is
 *   being sent over the web, you must use check_plain().
 */
function project_issue_change_summary($field, $value) {
  switch ($field) {
    case 'pid':
      $project = node_load(array('nid' => $value, 'type' => 'project_project'));
      return $project->title;
    case 'category':
      return $value ? project_issue_category($value, 0) : t('<none>');
    case 'priority':
      return $value ? project_issue_priority($value) : t('<none>');
    case 'rid':
      if ($value) {
        $release->nid = $value;
        if (module_exists('project_release')) {
          $release = project_release_load($release);
        }
        else {
          $release->version = t('Unknown');
        }
        return $release->version;
      }
      return t('<none>');
    case 'assigned':
      $user = user_load(array('uid' => $value));
      return (int) $value === 0 ? variable_get('anonymous', t('Anonymous')) : $user->name;
    case 'sid':
      return $value ? project_issue_state($value) : t('<none>');
    default:
      return $value;
  }
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
