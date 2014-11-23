<?php
// $Id: project_issue.views.inc,v 1.1.2.6 2009/01/07 20:48:32 dww Exp $

/**
 * @file
 * Provides support for Views integration.
 *
 * This file currently defines two default views which replicate much of
 * the functionality currently present in the project_issue module.  However,
 * due to limitations of Views 1, it's not possible to completely replace
 * the views-like code in the project_issue module with actual views.
 *
 * Instead, the code here is designed to make it possible for one to create
 * views using project and project_issue fields.
 */

/**
 * Implementation of hook_views_tables().
 */
function _project_issue_views_tables() {
  // It is not strictly necessary to include $tables['node'] below,
  // however if we use the default Node: Changed time field provided
  // by the Views module the changed time is printed as "<time> ago".
  // Since the 'ago' takes up extra space, we provide our own field
  // here that uses a custom handler so that we can omit 'ago'.
  $tables['pi_node'] = array(
    'name' => 'node',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      )
    ),
    'fields' => array(
      'changed' => array(
        'name' => t('Project issue: Updated Time'),
        'sortable' => true,
        'handler' => views_handler_field_project_issue_since,
        'option' => 'string',
        'help' => t('Display the last time the issue node was updated.'),
      ),
    ),
  );

  $tables['project_issues'] = array(
    'name' => 'project_issues',
    'join' => array(
      'type' => 'inner',
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
    ),
    'fields' => array(
      // To be useful, 'pid' requires another JOIN on {node}, so see
      // the project_issue_project_node definition below.
      'category' => array(
        'name' => t('Project Issue: Category'),
        'sortable' => true,
        'help' => t('The issue\'s category (bug, task, feature, etc).'),
      ),
      'component' => array(
        'name' => t('Project Issue: Component'),
        'sortable' => true,
        'help' => t('The issue\'s component (the options are controlled per-project).'),
      ),
      'priority' => array(
        'name' => t('Project Issue: Priority'),
        'handler' => 'views_handler_field_project_issue_priority',
        'sortable' => true,
        'help' => t('The issue\'s priority (critical, normal, minor).'),
      ),
      'rid' => array(
        'name' => t('Project Issue: Version'),
        'option' => array(
           '#type' => 'select',
           '#options' => array(
             'project_issue_rid_always_show' => t('Always display this field'),
             'project_issue_rid_allow_hide' => t('Allow this field to be hidden'),
            ),
        ),
        'query_handler' => 'views_query_handler_field_project_issue_version',
        'handler' => 'views_handler_field_project_issue_version',
        'sortable' => true,
        'help' => t('The version associated with the issue (depends on project_release.module). Choose "Always display this field" if you want this field to be displayed in the view even if no project is selected.'),
      ),
      'sid' => array(
        'name' => t('Project Issue: Status'),
        'help' => t('The status of each issue.'),
        'sortable' => true,
        'handler' => 'views_handler_field_project_issue_status',
      ),
      'all_files' => array(
        'name' => t('Project Issue: All files for issue'),
        'notafield' => true,
        'query_handler' => 'views_query_handler_project_issue_all_files',
        'handler' => array(
          'views_handler_project_issue_all_files' => t('All files'),
          'views_handler_project_issue_listed_files' => t('Listed files')),
        'option' => array(
          '#type' => 'select',
          '#options' => array(
            'link' => t('File names with links'),
            'nolink' => t('File names without links'),
            'linkdesc' => t('File descriptions with links'),
            'nolinkdesc' => t('File descriptions without links'),
          )),
        'sortable' => false,
        'help' => t('Display all files attached to an issue or comments on an issue in one field.'),
      ),
     ),
    'filters' => array(
      'category' => array(
        'name' => t('Project Issue: Category'),
        'operator' => 'views_handler_operator_or',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => project_issue_category(),
        ),
        'value-type' => 'array',
        'help' => t('Filter by issue category.'),
      ),
      'priority' => array(
        'name' => t('Project Issue: Priority'),
        'operator' => 'views_handler_operator_or',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => project_issue_priority(),
        ),
        'value-type' => 'array',
        'help' => t('Filter by issue priority.'),
      ),
      'sid' => array(
        'name' => t('Project Issue: Status'),
        'operator' => 'views_handler_operator_or',
        'value-type' => 'array',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => array('defaults' => implode(',', project_issue_state(0, false, false, 0, true))) + project_issue_state(),
        ),
        'value-type' => 'array',
        'handler' => 'views_handler_filter_project_issue_status_filter',
        'help' => t('Filter by issue status.'),
      ),
      'assigned' => array(
        'name' => t('Project Issue: Assigned to a given user'),
        'operator' => 'views_handler_operator_or',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => 'views_handler_filter_username',
        ),
        'value-type' => 'array',
        'handler' => 'views_handler_filter_project_issue_assigned',
        'help' => t('This allows you to filter by whether or not the issue is assigned to a certain user.'),
      ),
      'participant' => array(
        'name' => t('Project Issue: A given user has participated in an issue'),
        'operator' => 'views_handler_operator_or',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => 'views_handler_filter_username',
        ),
        'value-type' => 'array',
        'handler' => 'views_handler_filter_project_issue_participant',
        'help' => t('Filter whether a user has participated in an issue by creating the issue or commenting on it.'),
      ),
      'has_attachment' => array(
        'name' => t('Project Issue: Issue or comment has attachment'),
        'operator' => array('=' => t('Exists')),
        'value' => array(
          '#type' => 'select',
          '#options' => 'views_handler_operator_yesno',
        ),
        'handler' => 'views_handler_filter_project_issue_fid_exist',
        'help' => t('Filter whether the issue or a comment on the issue has an attachment.'),
      ),
    ),
  );

  // Add data from the project module
  $uris = NULL;
  $tables['project_issue_project_node'] = array(
    'name' => 'node',
    'join' => array(
      'type' => 'inner',
      'left' => array(
        'table' => 'project_issues',
        'field' => 'pid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
    ),
    'fields' => array(
      'title' => array(
        'name' => t('Project Issue Project Node: Title'),
        'handler' => array(
          'views_handler_field_project_issue_project_nodelink' => t('Normal'),
          'views_handler_field_project_issue_project_nodelink_with_mark' => t('With updated mark')
        ),
        'option' => array(
           '#type' => 'select',
           '#options' => array(
             'link' => t('As link to project node'),
             'nolink' => t('Without link')
            ),
        ),
        'sortable' => true,
        'pid' => 'nid',
        'addlfields' => array('changed', 'nid'),
        'help' => t('Display the title of the project.'),
      ),
    ),
    'filters' => array(
      'nid' => array(
        'name' => t('Project Issue Project Node: Project'),
        'operator' => 'views_handler_operator_or',
        'value' => array(
          '#type' => 'select',
          '#multiple' => TRUE,
          '#options' => project_projects_select_options($uris),
        ),
        'value-type' => 'array',
        'help' => t('Filter issues by project.  If the user has selected a project to filter by, project names will not be displayed.'),
      ),
    ),
  );

  // Add data from {project_projects}
  $tables['project_issue_project_projects'] = array(
    'name' => 'project_projects',
    'join' => array(
      'type' => 'inner',
      'left' => array(
        'table' => 'project_issues',
        'field' => 'pid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
    ),
    'fields' => array(
      'uri' => array(
        'name' => t('Project Issue Project Node: Short name'),
        'sortable' => true,
        'help' => t('Display the short name of the project.  In most cases you will want to include this field in your view but select the "Do not display this field" option.'),
        'option' => array(
           '#type' => 'select',
           '#options' => array(
             'project_issue_display' => t('Display this field'),
             'project_issue_nodisplay' => t('Do not display this field')
           ),
        ),
      ),
    ),
  );

  // Add data from {users} for issue assignment.
  $tables['pi_users'] = array(
    'name' => 'users',
    'join' => array(
      'type' => 'inner',
      'left' => array(
        'table' => 'project_issues',
        'field' => 'assigned'
      ),
      'right' => array(
        'field' => 'uid'
      ),
    ),
    'fields' => array(
      'name' => array(
        'name' => t('Project Issue: Assigned'),
        'handler' => 'views_handler_field_project_issue_assigned',
        'sortable' => true,
        'uid' => 'uid',
        'addlfields' => array('uid', 'name'),
        'help' => t('The user a given issue is assigned to.'),
      ),
    ),
  );
  return $tables;
}

/**
 * Displays a field indicating the status of an issue.
 */
function views_handler_field_project_issue_status($fieldinfo, $fielddata, $value, $data) {
  return check_plain(project_issue_state($value));
}

/**
 * Displays a field indicating the priority of an issue.
 */
function views_handler_field_project_issue_priority($fieldinfo, $fielddata, $value, $data) {
  return check_plain(project_issue_priority($value));
}

function views_query_handler_project_issue_all_files($field, $fieldinfo, &$query) {
  $query->add_field('vid', 'node');
}

/**
 * Display all files attached to a given node.
 */
function views_handler_project_issue_all_files($fieldinfo, $fielddata, $value, $data, $listed = false) {
  $and = $listed ? ' AND list = 1' : '';
  $links = array();

  // There is likely a more efficient way to do this that uses only one query, but I
  // don't know that that is.
  $result = db_query("SELECT f.*, fr.* FROM {file_revisions} fr INNER JOIN {files} f ON f.fid = fr.fid WHERE fr.vid = %d $and", $data->vid);
  while ($file = db_fetch_object($result)) {
    // link/nolink use file filename; linkdesc/nolinkdesc use file description
    if ($fielddata['options'] == 'link' || $fielddata['options'] == 'nolink') {
      $display_string = $file->filename;
    } else {
      $display_string = $file->description;
    }
    if ($fielddata['options'] == 'nolink' || $fielddata['options'] == 'nolinkdesc') {
      $links[] = check_plain($display_string);
    } else {
      $links[] = l($display_string, check_url(file_create_url($file->filepath)));
    }
  }
  $result = db_query("SELECT * FROM {comment_upload_files} WHERE nid = %d $and", $data->nid);
  while ($file = db_fetch_object($result)) {
    // link/nolink use file filename; linkdesc/nolinkdesc use file description
    if ($fielddata['options'] == 'link' || $fielddata['options'] == 'nolink') {
      $display_string = $file->filename;
    } else {
      $display_string = $file->description;
    }
    if ($fielddata['options'] == 'nolink' || $fielddata['options'] == 'nolinkdesc') {
      $links[] = check_plain($display_string);
    } else {
      $links[] = l($display_string, check_url(file_create_url($file->filepath)));
    }
  }
  return implode(' | ', $links);
}

function views_handler_project_issue_listed_files($fieldinfo, $fielddata, $value, $data) {
  return views_handler_project_issue_all_files($fieldinfo, $fielddata, $value, $data, TRUE);
}

function views_handler_filter_project_issue_fid_exist($op, $filter, $filterdata, &$query) {
  switch ($op) {
    case 'handler':
      $query->ensure_table('file_revisions');
      $query->ensure_table('comment_upload_files');
      if ($filter['value']) {
        $query->add_where('file_revisions.fid OR comment_upload_files.fid');
      }
      else {
        $query->add_where('ISNULL(file_revisions.fid) AND ISNULL(comment_upload_files.fid)');
      }
  }
}

function views_handler_filter_project_issue_participant($op, $filter, $filterdata, &$query) {
  switch ($op) {
    case 'handler':
      $query->ensure_table('comments');
      if ($filter['value']) {
        $uids = $filter['value'];
        if ($filter['operator'] == 'OR') {
          $query->add_where("node.uid IN (". implode(',', array_fill(0, count($uids), '%d')) .") OR comments.uid IN (". implode(',', array_fill(0, count($uids), '%d')) .")", array_merge($uids, $uids));
        }
        elseif ($filter['operator']  == 'NOR') {
          $query->add_where("node.uid NOT IN (". implode(',', array_fill(0, count($uids), '%d')) .")", $uids);
          $query->add_where("comments.uid NOT IN (". implode(',', array_fill(0, count($uids), '%d')) .")", $uids);
        }
    }
  }
}

function views_handler_filter_project_issue_assigned($op, $filter, $filterdata, &$query) {
  switch ($op) {
    case 'handler':
      $query->ensure_table('project_issues');
      if ($filter['value']) {
        $uids = $filter['value'];
        if ($filter['operator'] == 'OR') {
          $query->add_where("project_issues.assigned IN (". implode(',', array_fill(0, count($uids), '%d')) .")", $uids);
        }
        elseif ($filter['operator']  == 'NOR') {
          $query->add_where("project_issues.assigned NOT IN (". implode(',', array_fill(0, count($uids), '%d')) .")", $uids);
        }
    }
  }
}

/**
 * Format the assigned field as a username.
 * Loosely based on views_handler_field_username, but doesn't return
 * "Anonymous" when there is no one assigned.
 */
function views_handler_field_project_issue_assigned($fieldinfo, $fielddata, $value, $data) {
  $name = '';
  $obj = new stdClass();
  $uidfield = $fielddata['tablename'] . "_"  . $fieldinfo['uid'];
  if (is_numeric($data->pi_users_uid) && $data->pi_users_uid != 0) {
    // Loading each user seems expensive here, so just pass
    // enough information for theme_username() to work properly.
    $obj = new stdClass();
    $obj->uid = $data->pi_users_uid;
    $obj->name = $data->pi_users_name;
    $name = theme('username', $obj);
  }
  else {
    $name = '';
  }
  return $name;
}

/**
 * Format the assigned field as a version string.
 */
function views_handler_field_project_issue_version($fieldinfo, $fielddata, $value, $data) {
  if (!empty($data->project_issues_rid)) {
    $release = node_load($data->project_issues_rid);
    if (!empty($release)) {
      $version = project_release_get_version($release, $project = NULL);
      if (!empty($version)) {
        return $version;
      }
    }
  }
}

/*
 * Format a project title to link to the project or to the project's issue queue.
 */
function views_handler_field_project_issue_project_nodelink($fieldinfo, $fielddata, $value, $data) {
  switch ($fielddata['options']) {
    case 'nolink':
      return check_plain($value);
      break;
    case 'link':
      return l($value, "node/$data->project_issue_project_node_nid");
  }
}

/*
 * Format a project title as a link to the project node with a 'mark' stating whether or not the project has
 * updated since it was last viewed by the user.
 */
function views_handler_field_project_issue_project_nodelink_with_mark($fieldinfo, $fielddata, $value, $data) {
  switch ($fielddata['options']) {
    case 'nolink':
      $link = check_plain($value);
      break;
    case 'link':
      $link = l($value, "node/$data->project_issue_project_node_nid");
  }
  return $link .' '. theme('mark', node_mark($data->project_issue_project_node_nid, $data->project_issue_project_node_changed));
}

/**
 * Format a date as "X time".
 */
function views_handler_field_project_issue_since($fieldinfo, $fielddata, $value, $data) {
  return $data->node_changed ? t('!time', array('!time' => format_interval(time() - $data->node_changed, 2))) : theme('views_nodate');
}

/**
 * Handle special case for status filter
 *
 * Assumes filter value is an array and looks for special 'defaults' value in
 * array. If present, it adds default sids to the list of filter values.
 */
function views_handler_filter_project_issue_status_filter($op, $filter, $filterinfo, &$query) {
  $include = array();
  foreach ($filter['value'] as $val) {
    if ($val == 'defaults') {
      foreach (project_issue_default_states() as $default) {
        $include[] = $default;
      }
    } else {
      $include[] = $val;
    }
  }
  $filter['value'] = array_unique($include);
  return views_handler_filter_default($op, $filter, $filterinfo, $query);
}

/**
 * Implementation of hook_views_arguments().
 */
function _project_issue_views_arguments() {
  $arguments['project_shortname'] = array(
    'name' => t('Project issue: Project: short name'),
    'handler' => 'views_handler_arg_project_shortname',
    'help' => t('Filter issues by the project they belong to.'),
  );
  $arguments['project_issue_category'] = array(
    'name' => t('Project issue: Category'),
    'handler' => 'views_handler_arg_project_issue_category',
    'help' => t('Filter issues by their category (bug, feature, etc).'),
  );
  return $arguments;
}

function views_handler_arg_project_shortname($op, &$query, $argtype, $arg = '') {
  switch ($op) {
    case 'summary':
      $query->add_table('project_issue_project_projects', TRUE);
      $query->add_field('uri', 'project_issue_project_projects');
      $fieldinfo['field'] = 'project_issue_project_projects.uri';
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('project_issue_project_projects', 'uri', $argtype);
      break;
    case 'filter':
      $query->ensure_table('project_issue_project_projects');
      $query->add_where("project_issue_project_projects.uri = '%s'", $arg);
      break;
    case 'link':
      return l($query->uri, "$arg/" . $query->uri);
    case 'title':
      return check_plain($query);
  }
}


function views_handler_arg_project_issue_category($op, &$query, $argtype, $arg = '') {
  switch ($op) {
    case 'summary':
      $query->add_table('project_issues', TRUE);
      $query->add_field('category', 'project_issues');
      $fieldinfo['field'] = 'project_issues.category';
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('project_issues', 'category', $argtype);
      break;
    case 'filter':
      $query->ensure_table('project_issues');
      $query->add_where("project_issues.category = '%s'", $arg);
      break;
    case 'link':
      return l(project_issue_category($query->category), "$arg/" . $query->category);
    case 'title':
      return project_issue_category($query);
  }
}

/**
 * Implementation of hook_views_pre_view().
 */
function project_issue_views_pre_view(&$view) {
  // Remove any fields with option set to "project_issue_nodisplay"
  // from the view so they aren't displayed.  At the same time
  // this creates an associative array with field name as the
  // key and index as the value.  This array may be used later on
  // in the function, depending on which view is being used.
  $view_fields = array();
  foreach ($view->field as $index => $field) {
    $view_fields[$field['id']] = $index;
    if ($field['options'] == 'project_issue_nodisplay') {
      unset($view->field[$index]);
      unset($view->table_header[$index]);
    }
  }

  // If this view is one defined in this file, we need alter it some.  Otherwise,
  // leave it alone and just return.
  switch ($view->name) {
    case 'project_issues_project':
      // Set the proper title for the page.
      $name = 'all projects';
      // Try to find the project, and give a 404 error if it is not found.
      $project = NULL;
      $project_argument = $view->used_filters['filter0'];
      if (!empty($project_argument) && $project_argument != '**ALL**') {
        $project = project_project_retrieve($project_argument);
        if (!empty($project->title)) {
          $name = $project->title;
        }
        else {
          drupal_not_found();
          exit;
        }
      }
      break;

    case 'project_issues_project_search':
      //dsm($view);
     // Set the proper title for the page.
      $name = 'all projects';
      // Try to find the project, and give a 404 error if it is not found.
      $project = NULL;
      $project_argument = $view->used_filters['filter1'];
      if (count($project_argument) == 1 && $project_argument[0] != '**ALL**') {
        $project = project_project_retrieve($project_argument[0]);
        if (!empty($project->title)) {
          $name = $project->title;
        }
        else {
          drupal_not_found();
          exit;
        }
      }
      break;

    default:
      return;
  }

  // Remove the Project column if we're filtering by a specific project already
  if (isset($project)) {
    unset($view->field[$view_fields['project_issue_project_node.title']]);
    unset($view->table_header[$view_fields['project_issue_project_node.title']]);
  }

  // Remove the version column unless we're filtering by a specific project that has releases or
  // the version field is set to always be displayed.
  if (empty($project->releases) && $view->field[$view_fields['project_issues.rid']]['options'] == 'project_issue_rid_allow_hide') {
    unset($view->field[$view_fields['project_issues.rid']]);
    unset ($view->table_header[$view_fields['project_issues.rid']]);
  }

  global $user;
  $output = '';

  // If this view is a page, then set the title and breadcrumb, and add the links
  // for creating a new issue, viewing statistics, subscribing,
  // and advanced search to the top of the page.
  if ($view->build_type == 'page') {
    // Set the page title.
    if (!empty($view->args[0]) && $view->args[0] == 'feed') {
      $title = t('Issues for @projects', array('@projects' => $name));
    }
    else {
      $title = t('Issues for %projects', array('%projects' => $name));
    }
    $view->page_title = $title;

    project_project_set_breadcrumb($project, TRUE);

    if (isset($project)) {
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
        'href' => "project/view-issues/search/$project->uri",
        'attributes' => array('title' => t('Use the advanced search page to find @project issues.', array('@project' => $project->uri))),
      );
    }
    else {
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
        'href' => "project/view-issues/search",
        'attributes' => array('title' => t('Use the advanced search page for finding issues.')),
      );
    }
    $output .= '<div class="quick-search">';
    $output .= theme('links', $links);
    $output .= '</div>';
  }
    return $output;
}

/**
 * Implementation of hook_views_post_query().
 */
function project_issue_views_post_view(&$view) {
  // Create a permalink to this view so it can be bookmarked easily.
  // This code was taken and modified slightly from
  // views_rss_views_feed_argument() in views_rss.module.
  $args = array();
  $url = views_get_url($view, $args);
  $title = views_get_title($view, 'page', $args);

  if ($view->used_filters) {
    $filters = drupal_query_string_encode($view->used_filters);
  }
  $permalink = l(t('@project_issue_view_permalink', array('@project_issue_view_permalink' => '#')), $url, array('title' => t('Permalink to: !title', array('!title' => strip_tags($title)))), $filters, NULL, TRUE);
  return $permalink;
}

/**
 * Implementation of hook_views_style_plugins().
 */
function project_issue_views_style_plugins() {
  $items['project_issues'] = array(
    'name' => t('Project issues table'),
    'theme' => 'project_issues_table_view',
    'validate' => 'views_ui_plugin_validate_table',
    'needs_fields' => TRUE,
    'needs_table_header' => TRUE,
    'weight' => 1,
  );
  return $items;
}

function theme_project_issues_table_view($view, $nodes, $type) {
  $fields = _views_get_fields();
  foreach ($nodes as $node) {
    $class = "state-$node->project_issues_sid";
    $row_data = array();
    foreach ($view->field as $field) {
      if ($fields[$field['id']]['visible'] !== FALSE) {
        $cell['data'] = views_theme_field('views_handle_field', $field['queryname'], $fields, $field, $node, $view);
        $cell['class'] = "view-field ". views_css_safe('view-field-'. $field['queryname']);
        $row_data[] = $cell;
      }
    }
    $row_data = array('data' => $row_data, 'class' => $class);
    $rows[] = $row_data;
  }
  $output = '<div class="project-issue">';
  $output .= theme('table', $view->table_header, $rows);
  $output .= '</div>';
  return $output;
}

/**
 * Implementations of hook_views_default_views
 */
function _project_issue_views_default_views() {
  $view = new stdClass();
  $view->disabled = TRUE;
  $view->name = 'project_issues_project';
  $view->description = 'View issues of all projects or a particular project.';
  $view->access = array ();
  $view->view_args_php = '';
  $view->page = TRUE;
  $view->page_title = '';
  $view->page_header = '';
  $view->page_header_format = '1';
  $view->page_footer = '';
  $view->page_footer_format = '1';
  $view->page_empty = 'There are currently no issues to display for the criteria selected.';
  $view->page_empty_format = '1';
  $view->page_type = 'project_issues';
  $view->url = 'project/view-issues';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '50';
  $view->menu = TRUE;
  $view->menu_title = 'Project issues';
  $view->menu_tab = FALSE;
  $view->menu_tab_weight = '0';
  $view->menu_tab_default = FALSE;
  $view->menu_tab_default_parent = NULL;
  $view->menu_tab_default_parent_type = 'tab';
  $view->menu_parent_tab_weight = '0';
  $view->menu_parent_title = '';
  $view->sort = array ();
  $view->argument = array (
    array (
      'type' => 'rss_feed',
      'argdefault' => '2',
      'title' => '',
      'options' => '',
      'wildcard' => '',
      'wildcard_substitution' => '',
    ),
  );
  $view->field = array (
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'title',
      'label' => 'Project',
      'handler' => 'views_handler_field_project_issue_project_nodelink',
      'sortable' => '1',
      'options' => 'link',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => 'Summary',
      'handler' => 'views_handler_field_nodelink_with_mark',
      'options' => 'link',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'label' => 'Status',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'label' => 'Priority',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'label' => 'Category',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'rid',
      'label' => 'Version',
      'sortable' => '1',
      'options' => 'project_issue_rid_allow_hide',
    ),
    array (
      'tablename' => 'pi_node',
      'field' => 'changed',
      'label' => 'Last updated',
      'sortable' => '1',
      'defaultsort' => 'DESC',
    ),
    array (
      'tablename' => 'pi_users',
      'field' => 'name',
      'label' => 'Assigned to',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issue_project_projects',
      'field' => 'uri',
      'label' => 'Short name',
      'options' => 'project_issue_nodisplay',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'nid',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'distinct',
      'operator' => '=',
      'options' => '',
      'value' => array (
        0 => 'distinct',
      ),
    ),
  );
  $view->exposed_filter = array (
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'nid',
      'label' => 'Project',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '1',
      'single' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'label' => 'Status',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '1',
      'single' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'label' => 'Category',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '1',
      'single' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'label' => 'Priority',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '1',
      'single' => '1',
    ),
  );
  $view->requires = array(project_issue_project_node, node, project_issues, pi_node, pi_users, project_issue_project_projects);
  $views[$view->name] = $view;

  $view = new stdClass();
  $view->disabled = TRUE;
  $view->name = 'project_issues_project_search';
  $view->description = 'Search for issues by many fields.';
  $view->access = array ();
  $view->view_args_php = '';
  $view->page = TRUE;
  $view->page_title = '';
  $view->page_header = '';
  $view->page_header_format = '1';
  $view->page_footer = '';
  $view->page_footer_format = '1';
  $view->page_empty = 'There are currently no issues to display for the criteria selected.';
  $view->page_empty_format = '1';
  $view->page_type = 'project_issues';
  $view->url = 'project/view-issues/search';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '50';
  $view->menu = TRUE;
  $view->menu_title = 'Project issues';
  $view->menu_tab = FALSE;
  $view->menu_tab_weight = '0';
  $view->menu_tab_default = FALSE;
  $view->menu_tab_default_parent = NULL;
  $view->menu_tab_default_parent_type = 'tab';
  $view->menu_parent_tab_weight = '0';
  $view->menu_parent_title = '';
  $view->sort = array ();
  $view->argument = array (
    array (
      'type' => 'rss_feed',
      'argdefault' => '2',
      'title' => '',
      'options' => '',
      'wildcard' => '',
      'wildcard_substitution' => '',
    ),
  );
  $view->field = array (
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'title',
      'label' => 'Project',
      'handler' => 'views_handler_field_project_issue_project_nodelink',
      'sortable' => '1',
      'options' => 'link',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => 'Summary',
      'handler' => 'views_handler_field_nodelink_with_mark',
      'options' => 'link',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'label' => 'Status',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'label' => 'Priority',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'label' => 'Category',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'rid',
      'label' => 'Version',
      'sortable' => '1',
      'options' => 'project_issue_rid_allow_hide',
    ),
    array (
      'tablename' => 'pi_node',
      'field' => 'changed',
      'label' => 'Last updated',
      'sortable' => '1',
      'defaultsort' => 'DESC',
    ),
    array (
      'tablename' => 'pi_users',
      'field' => 'name',
      'label' => 'Assigned to',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'project_issue_project_projects',
      'field' => 'uri',
      'label' => 'Short name',
      'options' => 'project_issue_nodisplay',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'nid',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'operator' => 'OR',
      'options' => '',
      'value' => array (),
    ),
    array (
      'tablename' => 'users',
      'field' => 'uid',
      'operator' => 'OR',
      'options' => '',
      'value' => array (
        0 => '1',
      ),
    ),
    array (
      'tablename' => 'temp_search_results',
      'field' => 'word',
      'operator' => '=',
      'options' => '',
      'value' => '',
    ),
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'distinct',
      'operator' => '=',
      'options' => '',
      'value' => array (
        0 => 'distinct',
      ),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'has_attachment',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'participant',
      'operator' => 'OR',
      'options' => '',
      'value' => array (
        0 => '1',
      ),
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'assigned',
      'operator' => 'OR',
      'options' => '',
      'value' => array (
        0 => '1',
      ),
    ),
  );
  $view->exposed_filter = array (
    array (
      'tablename' => 'temp_search_results',
      'field' => 'word',
      'label' => 'Title, issue, or comment',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issue_project_node',
      'field' => 'nid',
      'label' => 'Project',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'sid',
      'label' => 'Status',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'category',
      'label' => 'Category',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'priority',
      'label' => 'Priority',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'users',
      'field' => 'uid',
      'label' => 'Submitted by',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'has_attachment',
      'label' => 'Has attachment',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '1',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'participant',
      'label' => 'Participant',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
    array (
      'tablename' => 'project_issues',
      'field' => 'assigned',
      'label' => 'Assigned',
      'optional' => '1',
      'is_default' => '0',
      'operator' => '0',
      'single' => '0',
    ),
  );
  $view->requires = array(project_issue_project_node, node, project_issues, pi_node, pi_users, project_issue_project_projects, users, temp_search_results);
  $views[$view->name] = $view;

  return $views;
}
