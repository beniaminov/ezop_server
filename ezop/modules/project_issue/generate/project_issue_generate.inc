<?php
// $Id: project_issue_generate.inc,v 1.1.2.3 2008/01/26 02:53:48 dww Exp $

// If not in 'safe mode', increase the maximum execution time :
if (!ini_get('safe_mode')) {
  set_time_limit(240);
}

/**
 * Generate some random project issues.
 *
 * @param $num
 *  Number of issues to generate.
 */
function project_issue_generate_issues($num) {
  require_once(drupal_get_path('module', 'devel') .'/devel_generate.inc');

  $projects = _project_issue_generate_get_field('projects');
  $categories = _project_issue_generate_get_field('categories');
  $priorities = _project_issue_generate_get_field('priorities');
  $users = _project_issue_generate_get_field('users');
  $loaded_users = array();

  for ($i = 0; $i < $num; $i++) {
    srand((double) microtime() * 1000000);

    $project = $projects[array_rand($projects)];
    $components = unserialize($project->components);
    $issue = array();
    $issue['pid'] = $project->nid;
    $issue['category'] = $categories[array_rand($categories)];
    $issue['component'] = $components[array_rand($components)];
    $issue['priority'] = array_rand($priorities);
    $issue['title'] = devel_create_greeking(rand(2, 15), true);
    $issue['body'] = devel_create_content();

    // The user must be chosen before status (sid) so that we can make sure
    // that the status is set such that the user would have permission to
    // set the status as such.
    $account = user_load(array('uid' => $users[array_rand($users)]->uid));
    $issue['name'] = $account->name;
    $issue['sid'] = array_rand(_project_issue_generate_get_permitted_sids($account));
    if (!isset($issue['sid'])) {
      unset($issue);
      continue;
    }

    // Currently, project_issue nodes can either be unassigned or assigned
    // to the user creating the project_issue node (or in the case of comments
    // the user creating the comment).
    $possible_assignments = array(0, $account->uid);
    $assigned = $possible_assignments[array_rand($possible_assignments)];

    drupal_execute('project_issue_node_form', $issue, array('type' => 'project_issue', 'pid' => $issue['pid'], 'assigned' => $assigned));
  }
}

function _project_issue_generate_get_field($field, $pool_size = 100) {
  require_once(drupal_get_path('module', 'project_issue') . '/issue.inc');

  switch ($field) {
    case 'projects':
      $projects = array();

      $result = db_query('SELECT nid, components FROM {project_issue_projects}');
      while ($project = db_fetch_object($result)) {
        $projects[] = $project;
      }

      return $projects;

    case 'categories':
      $categories = array_keys(project_issue_category());
      return $categories;

    case 'priorities':
      $priorities = project_issue_priority();
      return $priorities;

    case 'users':
      // Determine what role ids have permission to create project_issue nodes.
      $users = array();
      $allowed_roles = user_roles(FALSE, 'create project issues');

      // If any authenticated user can create project_issue nodes,
      // then there is no need for an INNER JOIN in our query.
      // Otherwise, the query needs to INNER JOIN on the users
      // table so that only users with roles that are allowed to
      // create project_issue nodes are selected.
      if (isset($allowed_roles[DRUPAL_AUTHENTICATED_RID])) {
        $join = '';
        $where = '';
      }
      else {
        $join = 'INNER JOIN {users_roles} ur ON u.uid = ur.uid';
        $where = "WHERE ur.rid IN (". implode(', ', array_keys($allowed_roles)) .")";
      }
      $sql = "SELECT u.uid FROM {users} u $join $where ORDER BY RAND() LIMIT %d";
      $result = db_query($sql, $pool_size);
      while ($user = db_fetch_object($result)) {
        $users[] = $user;
      }
      return $users;
  }
}

function _project_issue_generate_get_permitted_sids($user) {
  static $permitted_states;

  if (!isset($permitted_states)) {
    $permitted_states = array();
  }

  if (isset($user->uid)) {
    if (isset($permitted_states[$user->uid])) {
      return $permitted_states[$user->uid];
    }
    else {
      $states = project_issue_state($sid = 0, TRUE, TRUE, 0, FALSE, $user);
      $permitted_states[$user->uid] = $states;
      return $states;
    }
  }
  return array();
}
