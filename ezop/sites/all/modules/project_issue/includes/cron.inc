<?php
// $Id: cron.inc,v 1.1 2008/12/28 23:32:43 dww Exp $


/**
 * @file
 * Contains the code required during cron runs for periodic functionality.
 *
 * This code is either invoked via hook_cron() or project-issue-cron.php 
 * depending on the value of the 'project_issue_hook_cron' variable.
 */

/**
 * Private helper function to run periodic functionality.
 *
 * This code is either invoked by hook_cron() or via project-issue-cron.php
 * depending on the value of the 'project_issue_hook_cron' variable. It is
 * responsible for auto-closing issues, and sending email digests and
 * reminders.
 *
 * @see project_issue_cron()
 */
function _project_issue_cron() {
  if (time() - variable_get('project_issue_digest_last', 0) > variable_get('project_issue_digest_interval', 7 * 24 * 60 * 60)) {
    variable_set('project_issue_digest_last', time());
    project_mail_digest();
  }

  if (time() - variable_get('project_issue_reminder_last', 0) > variable_get('project_issue_reminder_interval', 28 * 7 * 24 * 60 * 60)) {
    variable_set('project_issue_reminder_last', time());
    project_mail_reminder();
  }

  // Auto-close fixed issues;
  project_issue_auto_close();
}

/**
 * Automatically close issues marked as fixed for a specified number of days
 * and add a comment to each documenting the change.
 */
function project_issue_auto_close() {
  // Set query parameters.
  $seconds = 24 * 60 * 60 * variable_get('project_issue_auto_close_days', PROJECT_ISSUE_AUTO_CLOSE_DAYS);

  $comment = theme('project_issue_auto_close_message');
  $result = db_query('SELECT pi.nid FROM {project_issues} pi INNER JOIN {node} n ON n.nid = pi.nid WHERE pi.sid = %d AND n.changed < %d', PROJECT_ISSUE_STATE_FIXED, time() - $seconds);
  while ($issue = db_fetch_object($result)) {
    project_issue_add_followup(array(
      'nid' => $issue->nid,
      'sid' => PROJECT_ISSUE_STATE_CLOSED,
      'comment' => $comment,
    ));
  }
}

function project_mail_reminder() {
  global $base_url;

  if (defined('PROJECT_NOMAIL')) {
    return;
  }

  $projects = array();
  $result = db_query(db_rewrite_sql('SELECT p.nid, n.title FROM {project_issue_projects} p INNER JOIN {node} n ON p.nid = n.nid WHERE p.mail_reminder = 1 AND n.status = 1', 'p'));
  while ($project = db_fetch_object($result)) {
    $projects[$project->nid] = $project->title;
    $pids[] = "p.pid = $project->nid";
  }
  if (count($projects)) {

    $sender->name = variable_get('site_name', '');
    $sender->mail = variable_get('site_mail', '');

    $result = db_query(db_rewrite_sql('SELECT p.nid, n.*, p.*, u.name, u.mail FROM {project_issues} p INNER JOIN {node} n ON p.nid = n.nid INNER JOIN {users} u ON n.uid = u.uid WHERE n.status = 1 AND u.status = 1 AND ('. implode(' OR ', $pids) .") AND u.mail <> '' AND (p.sid = 1 OR p.sid = 2) ORDER BY u.uid, p.pid, p.component, p.sid, n.changed DESC", 'p'));

    while (($node = db_fetch_object($result)) || !empty($body)) {
      if ($body && $mail && ((!$node) || ($mail != $node->mail))) {
        $header = array(
          'Return-Path' => "<$sender->mail;>",
          'Date' => date('r'),
          'From' => "$sender->name <$sender->mail>",
          'X-Mailer' => 'Drupal Project Issues module (http://drupal.org/project/project_issue)',
          'List-Id' => "$sender->name <project-reminder-". preg_replace('|.+://([a-zA-Z0-9\._-]+).*|', '\1', $base_url) .'>',
          'List-Archive' => '<'. url('project', NULL, NULL, 1) .'>',
        );

        drupal_mail('project_issue_reminder', $mail, t('Your submitted bugs for !date', array('!date' => date('F d, Y', time()))), $body, NULL, $header);

        $body = '';
      }
      $mail = $node->mail;

      if ($node->nid) {
        if ($pid != $node->pid || $component != $node->component) {
          $pid = $node->pid;
          $component = $node->component;
          $banner = "$projects[$pid] / $component";
          $body .= "[ $banner ]". str_repeat('=', 72 - 4 - strlen($banner)) ."\n";
        }
        $body .= "$node->title\n";
        if ($node->assigned && $assigned = user_load(array('uid' => $node->assigned))) {
          $body .= "  assigned: $assigned->name\n";
        }
        $body .= '  state: '. project_issue_state($node->sid) ."\n";
        $body .= '  age: '. format_interval(time() - $node->created) ."\n";
        $body .= '  url: '. url("node/$node->nid", NULL, NULL, 1) ."\n";
        $body .= "\n";
      }
    }
  }
}

function project_mail_digest() {
  global $base_url;

  if (defined('PROJECT_NOMAIL')) {
    return;
  }

  $body = array();
  $header = array(
    'Date' => date('r'),
    'X-Mailer' => 'Drupal Project Issues module (http://drupal.org/project/project_issue)',
    'List-Id' => "$project->title <". preg_replace('|.+://([a-zA-Z0-9\._-]+).*|', '\1', $base_url) .'-project-issues-digest>',
    'List-Archive' => '<'. url('project/issues?priorities=1', NULL, NULL, 1) .'>',
  );

  $result = db_query(db_rewrite_sql("SELECT n.nid, n.title, p.* FROM {node} n INNER JOIN {project_issue_projects} p ON n.nid = p.nid WHERE n.status = 1 AND p.mail_digest <> '' ORDER BY n.title, p.mail_digest"));
  while ($project = db_fetch_object($result)) {
    $category = '';
    $result2 = db_query(db_rewrite_sql('SELECT p.nid, n.title, n.created, p.sid, p.category, p.component, p.priority, p.assigned FROM {project_issues} p INNER JOIN {node} n ON p.nid = n.nid WHERE n.status = 1 AND p.pid = %d AND p.sid = 1 AND p.priority = 1 ORDER BY p.category, n.created DESC', 'p'), $project->nid);
    while ($node = db_fetch_object($result2)) {
      if ($category != $node->category) {
        $category = $node->category;
        $banner = "$project->title / ". project_issue_category($node->category);
        $body[$project->mail_digest] .= "$banner\n". str_repeat('-', $banner) ."\n";
      }
      $body[$project->mail_digest] .= "$node->title\n";
      if ($node->assigned && $assigned = user_load(array('uid' => $node->assigned))) {
        $body[$project->mail_digest] .= '  assigned: '. $assigned->name ."\n";
      }
      $body[$project->mail_digest] .= '  age: '. format_interval(time() - $node->created) ."\n";
      $body[$project->mail_digest] .= '  url: '. url("node/$node->nid", NULL, NULL, 1) ."\n";
      $body[$project->mail_digest] .= "\n";
    }
  }

  foreach ($body as $mailto => $text) {
    $header['From'] = $mailto;
    $header['Return-Path'] = "<$mailto>";

    drupal_mail('project_issue_critical_summary', $mailto, t('Release critical bugs for !date', array('!date' => date('F d, Y', time()))), $text, NULL, $header);
  }
}

