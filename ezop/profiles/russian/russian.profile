<?php
// $Id: russian.profile,v 1.2 2007/03/14 20:27:58 vadbarsdrupalorg Exp $

/**
 * Russian Drupal installation profile
 *
 * Thanks to goba, the author of autolocale module
 */

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *  An array of modules to be enabled.
 */
function russian_profile_modules() {
// Core modules / Модули ядра
  $core = array('block', 'color', 'comment', 'filter', 'help', 'menu', 'node', 'system', 'taxonomy', 'user', 'watchdog', 'locale');

// Contrib modules / Дополнительные модули (можно добавлять или удалять по необходимости)
  $contrib = array('autolocale');

  return array_merge($core, $contrib);
}

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile.
 *   description: Выберите этот профиль для установки Русского Drupal.
 */
function russian_profile_details() {
  return array(
    'name' => 'Russian Drupal',
    'description' => '╨Т╤Л╨▒╨╡╤А╨╕╤В╨╡ ╤Н╤В╨╛╤В ╨┐╤А╨╛╤Д╨╕╨╗╤М ╨┤╨╗╤П ╤Г╤Б╤В╨░╨╜╨╛╨▓╨║╨╕ ╨а╤Г╤Б╤Б╨║╨╛╨│╨╛ Drupal.'
  );
}

/**
 * Uses functionality in autolocale.install to import PO files
 */
function russian_install() {
  _autolocale_install_po_files();
}

/**
 * Perform any final installation tasks for this profile.
 *
 * @return
 *   An optional HTML string to display to the user on the final installation
 *   screen.
 */
function russian_profile_final() {
  // Если сервер работает не в 'safe mode', увеличиваем максимальное время выполнения скриптов:
  if (!ini_get('safe_mode')) {
    set_time_limit(0);
  }

  // Переведем заголовок Primary links
  db_query("UPDATE {menu} SET title = '".st('Primary links')."' WHERE mid = '2'");

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

/** 
 * Configuration / Дополнительные настройки
 */

  // Sitename / Название сайта (кодировка UTF-8), например, "Русский Drupal"
  variable_set('site_name', '╨а╤Г╤Б╤Б╨║╨╕╨╣ Drupal');


  // Turn on user pictures / Включим аватары пользователей
//  variable_set('user_pictures', 1);


  // Register site admin / Создадим главного админа сайта и дадим ему все права
  // Имя: admin, пароль: admin, эл.почта: admin@mydrupalsite.ru
  // ВНИМАНИЕ!!! Не забудьте поменять эти настройки на рабочем сайте !!!
  db_query("INSERT INTO {users} (uid, name, pass, mail, created, status) VALUES(1, 'admin', '%s', 'admin@mydrupalsite.ru', %d, 1)", md5('admin'), time());
  user_authenticate('admin', 'admin');


  // Setup some standard roles (non-uid-1 admin, redactor (content admin) etc.),
  // and configure all perms appropriately.
  // Создадим  несколько стандартных ролей (админ сайта, редактор (управление содержанием) и т.п.)
  // и дадим соответствующие права доступа.
        //Примеры названий ролей (eng/rus/rus UTF-8):
        //redactor редактор ╤А╨╡╨┤╨░╨║╤В╨╛╤А
        //administrator администратор ╨░╨┤╨╝╨╕╨╜╨╕╤Б╤В╤А╨░╤В╨╛╤А
        //moderator модератор ╨╝╨╛╨┤╨╡╤А╨░╤В╨╛╤А
        //author автор ╨░╨▓╤В╨╛╤А

//  db_query("INSERT INTO {role} (rid, name) VALUES (3, '╨░╨┤╨╝╨╕╨╜╨╕╤Б╤В╤А╨░╤В╨╛╤А')");
//  db_query("INSERT INTO {role} (rid, name) VALUES (4, '╤А╨╡╨┤╨░╨║╤В╨╛╤А')");

  // Insert new role's permissions / Определим права для ролей
//  db_query("INSERT INTO {permission} (rid, perm, tid) VALUES (3, 'administer blocks, use PHP for block visibility, access comments, administer comments, post comments, post comments without approval, access devel information, execute php code, devel_node_access module, view devel_node_access information, administer filters, administer menu, access content, administer content types, administer nodes, create page content, create story content, edit own page content, edit own story content, edit page content, edit story content, revert revisions, view revisions, access administration pages, administer site configuration, select different theme, administer taxonomy, access user profiles, administer access control, administer users, change own username', 0)");
//  db_query("INSERT INTO {permission} (rid, perm, tid) VALUES (4, 'administer blocks, access comments, administer comments, post comments, post comments without approval, administer menu, access content, administer nodes, create page content, create story content, edit own page content, edit own story content, edit page content, edit story content, revert revisions, view revisions, access user profiles, administer users', 0)");


  // Change front page / Изменим главную страницу 
  // например на "user/register", чтобы сразу попадать на страницу регистрации первого пользователя-админа
  // или на любую другую страницу
  // (вернуть обычное "node" можно потом на странице admin/settings/site-information)
// variable_set('site_frontpage', 'user/register');

  // Set date and timezone settings
  // Региональные настройки даты и времени
  // Переменные:
  // Y	Порядковый номер года, 4 цифры	Примеры: 1999, 2003
  // y	Номер года, 2 цифры	Примеры: 99, 03
  // d  День месяца, 2 цифры с ведущими нулями	от 01 до 31
  // j	День месяца без ведущих нулей	От 1 до 31
  // F	Полное наименование месяца, например January или March	от January до December
  // M	Сокращенное наименование месяца, 3 символа	От Jan до Dec
  // m	Порядковый номер месяца с ведущими нулями	От 01 до 12
  // n	Порядковый номер месяца без ведущих нулей	От 1 до 12
  // g	Часы в 12-часовом формате без ведущих нулей	От 1 до 12
  // G	Часы в 24-часовом формате без ведущих нулей	От 0 до 23
  // h	Часы в 12-часовом формате с ведущими нулями	От 01 до 12
  // H	Часы в 24-часовом формате с ведущими нулями	От 00 до 23
  // i	Минуты с ведущими нулями	00 to 59
  // s	Секунды с ведущими нулями	От 00 до 59
  // D	Сокращенное наименование дня недели, 3 символа	от Mon до Sun
  // l (строчная 'L')	Полное наименование дня недели	От Sunday до Saturday
  variable_set('date_format_short', 'd.m.y'); // 01.12.07
  variable_set('date_format_medium', 'd.m.Y ╨▓ H:i'); // 01.12.2007 в 23:45
  variable_set('date_format_long', 'd F Y ╨│., H:i - l'); // 01 декабря 2007 г., 23:45 - среда
  // Первый день недели - понедельник
  variable_set('date_first_day', '1');

}
