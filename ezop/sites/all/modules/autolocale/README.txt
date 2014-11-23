
Autolocale module and install profile
================================================================================

The goal of the autolocale module and install profile is to provide a profile
to automatically import interface translations in install time and later when
modules are enabled / disabled.

How to use in installation time
--------------------------------------------------------------------------------

1. Create the profiles/localized folder.
2. Copy localized.profile into that folder.
3. Download a translation which is ready for Drupal 5 (ie. it has
   an installer.po file and other module translation files, eg. the
   Hungarian translation with the 'hu' language code)
4. Copy the installer.po into the 'localized' profile folder, and
   rename it to the language name, eg. hu.po for Hungarian
5. Move module translation files into the module folders (only
   translations of modules enabled by default will get imported),
   remember to name them by the language name (either hu.po or
   $filename.hu.po for example).
6. Start Drupal translation and use the "Drupal localized" profile
7. You should have the language enabled and the strings imported
   on your fully installed Drupal site.

How to use on an already installed site
--------------------------------------------------------------------------------

1. Copy autolocale module files to the modules folder (skip the
   localized.profile file). Putting the module into sites/all/modules/autolocale
   is suggested.
2. Turn on the module on the admin interface.
3. You will get guidance in the form of a Drupal message after enabling the
   module, so you will find the interface to automatically import translations
   on the web interface.
4. Later when you install or enable a module, all strings for that module
   will get imported for all enabled languages.

Information for developers
--------------------------------------------------------------------------------

Implemented installation workflow:

  - one selects the localized profile to install
  - selects the language to install with
  - the installer works in that language (this is a core feature already)
  - the install profile turns the locale module on
  - the install profile turns the autolocale module on,
    which searches for enabled modules/themes and imports their
    interface translations

Searched files are:

  - modulefolder/po/$languagecode.po
  - modulefolder/po/$anyname.$languagecode.po
  
The first is for backwards compatibility with contrib module practice. The
second allows for more language files for a language in one folder. This is
useful to separate the files by function (ie. system-install.hu.po,
system-module.hu.po and general.hu.po all go to modules/system/po).

TODO:

  - it would be very nice to find a way to remove unused translations
    (stuff that is only in locales_source or stuff related to modules
    already disabled).

Maintainer
--------------------------------------------------------------------------------

Concept, implementation and maintanance by Gabor Hojtsy (gabor at hojtsy.hu)
