<?php

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *  An array of modules to be enabled.
 */
function localized_profile_modules() {
  return array('block', 'color', 'comment', 'filter', 'help', 'menu', 'node', 'system', 'taxonomy', 'user', 'watchdog', 'locale', 'autolocale');
}

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile.
 */
function localized_profile_details() {
  return array(
    'name' => 'Drupal localized',
    'description' => 'Install basic Drupal functionality with interface translation.'
  );
}

/**
 * Perform any final installation tasks for this profile.
 * Most of this code is a copy of the core default profile.
 *
 * @return
 *   An optional HTML string to display to the user on the final installation
 *   screen.
 */
function localized_profile_final() {
  // Insert default user-defined node types into the database.
  $common = array(
    'module' => 'node',
    'custom' => TRUE,
    'modified' => TRUE,
    'locked' => FALSE,
    'has_body' => TRUE,
    'body_label' => st('Body'),
    'has_title' => TRUE,
    'title_label' => st('Title'),
  );
  $types = array(
    array_merge(
      array(
        'type' => 'page',
        'name' => st('Page'),
        'description' => st('If you want to add a static page, like a contact page or an about page, use a page.')
      ), 
      $common
    ),
    array_merge(
      array(
        'type' => 'story',
        'name' => st('Story'),
        'description' => st('Stories are articles in their simplest form: they have a title, a teaser and a body, but can be extended by other modules. The teaser is part of the body too. Stories may be used as a personal blog or for news articles.')
      ),
      $common
    ),
  );

  foreach ($types as $type) {
    $type = (object) _node_type_set_defaults($type);
    node_type_save($type);
  }

  // Default page to not be promoted and have comments disabled.
  variable_set('node_options_page', array('status'));
  variable_set('comment_page', COMMENT_NODE_DISABLED);

  // Don't display date and author information for page nodes by default.
  $theme_settings = variable_get('theme_settings', array());
  $theme_settings['toggle_node_info_page'] = FALSE;
  variable_set('theme_settings', $theme_settings);

  // Finally import all translations
  _autolocale_install_po_files();
}
