OVERVIEW
========

The "update status" module queries drupal.org to see if there are new
versions of Drupal core and any modules that you are running. It
produces an available updates report at admin/logs/updates:

Administer >> Logs >> Available updates

Every module is part of a "project", which may have the same name as
the module or a different name. Some projects (for example, Drupal
core itself) include more than one module. The report shows every
project, the currently installed version, what modules are included,
and information about any newer releases that are available. Each
entry in the available updates report includes information about what
kind of release it is, the version, date, and links to the release
notes and download page. Releases marked as security updates are
flagged with extra urgency.

If update_status finds that any of your modules are out of date, the
row for the project containing the module will be colored red and
marked as an error. At the system modules page (admin/build/modules)
there will be a warning message displayed. Also, the site-wide status
report (admin/logs/status) will mark this as an error. You can
configured update_status to only consider missing security updates an
error, and all other newer releases are just considered a warning
(colored yellow on the available updates report and status report, and
the error message at the system modules page will not be shown).

By default, update_status checks once a day (assuming cron is
correctly enabled). You can also check manually when viewing the
available updates page or the site-wide status report.


REQUIREMENTS
============

This module requires that the web server process (for example, httpd)
is able to initiate outbound connections. This is usually possible,
but some hosting providers or server configurations forbid such
connections.


INSTALLING THIS MODULE
======================

Put the entire update_status directory into your 'modules' directory
or your 'sites/all/modules' directory and visit admin/build/modules:

Administer >> Site building >> Modules

Enable the "Update status" module like any other.


CONFIGURING THIS MODULE
=======================

The settings page for update_status is a tab off the available updates
report at admin/logs/updates/settings:

Administer >> Logs >> Available updates >> Settings

There are settings to control how often update_status will check for
newer releases and what kinds of available releases should be
considered an error.

You can configure update_status not to warn about a project being out
of date. For each project that it identifies as having an official
release on drupal.org, you may tell it to warn if a project has a new
version available 'always', 'not for the current version', or
'never'. If you select 'not for the current version' and a new version
is posted, the project will then show as having an available update.
Modules that are set to not warn if updates are available, or that
lack enough information to compare, are displayed in yellow.


REPORTING USAGE STATISTICS
==========================

Whenever update_status queries drupal.org to check for available
updates, it can also provide anonymous information about your
installed modules. This data is used to generate statistics about the
actual usage of each module, which helps to improve the quality of
Drupal contributions and will eventually help users find and use
projects that are better maintained and more well-used. No information
that can be used to identify your site is sent or recorded, so there
is no need to be concerned about security or privacy.


UPGRADING TO 6.x CORE
=====================

This module has been moved into Drupal core 6.x and renamed the
"update.module". See http://drupal.org/node/94154 for background.
Sites using the contributed update_status module in 5.x should disable
*and* uninstall the module *before* upgrading to 6.x core. This will
prevent any conflicts during the upgrade, and will ensure that no
stale variables are left in your database which are not used by the
6.x version in core.

Please note that the settings to ignore specific releases or specific
projects are not included in the update.module in core, and if you
make use of this functionality on your site, you should install the
contributed "Update status advanced settings" module:

http://drupal.org/project/update_advanced


DEPLOYING FROM CVS
==================

If you deploy your Drupal site directly from CVS, please see the CVS
deploy module:

http://drupal.org/project/cvs_deploy

If you enable both update_status and cvs_deploy, the available updates
report will be much more accurate for the modules you install from CVS.


AUTHORS
=======

The original version of this module (5.x-1.*) was written by Nedjo
Rogers and Earl Miles, and the back-end server code at drupal.org
(part of the project.module) was mostly by Nedjo, Earl and Derek Wright.
The current version (5.x-2.*) was a fairly fundamental re-write of the
original, based on lessons learned in practice. The back-end server
infrastructure and protocol for transfering data about available
updates was completely changed by Derek, and the client code was
overhauled by Earl and Derek.

Nedjo Rogers: http://drupal.org/user/4481 (nedjo)
Earl Miles: http://drupal.org/user/26979 (merlinofchaos)
Derek Wright: http://drupal.org/user/46549 (dww)


$Id: README.txt,v 1.6.2.2 2008/01/22 04:13:19 dww Exp $
