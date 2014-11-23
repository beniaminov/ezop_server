Description:
------------
        
This is a simple site map module that provides two main functions:

- A 'site map' page giving a hierarchy overview of the site's taxonomy.
  Visitors to www.yoursite.com/sitemenu will see the various categories
  of your web site and (optionally) the nodes titles under each category.
  Taxonomy terms that are associated with image galleries link to the
  url as image/tid/xx, and those associated with forums link to the url
  as forum/xx.

- A side bar menu to navigate your web site by category, when configured
  as a block.

Features:
---------

This module provides several options that customize its look and feel:

* An overview section on the top of the page can contain any text you want
* Can exclude certain taxonomies from the site map page.
* Can show the number of nodes under each taxonomy term.
* Can list the nodes under each taxonomy term, with hyperlinks for each
* If taxonomies have descriptions, then a mouseover description is shown
* Can list the author and number of comments for each node, also by a mouseover
* Can be configured as a block, providing a side menu for navigation.

Note on the block feature:
  Please note that this taxonomy is not collapsible as in the taxonomy_context
  module. This means it does take more screen space if your taxonomy is multi
  level and contains lots of terms. The advantage is that it does not require
  multiple clicks for the user to get to the information they may be interested
  in. Also, the menu is always visible, not when you are on a node only, as in
  taxonomy_context

Database:
---------
This module does not require any new database tables to be installed.

Installation:
-------------

Please see the INSTALL document for details.

Bugs/Features/Patches:
----------------------

If you want to report bugs, feature requests, or submit a patch, please do so
at the project page on the Drupal web site.
http://drupal.org/node/16130

Author
------

Based on the taxonomy_dhtml module which was adapted for Drupal
by Moshe Weitzman <weitzman AT tejasa.com>, who based it on work
by Gazingus at http://www.gazingus.org/dhtml/?id=109.

Rewritten by: Khalid Baheyeldin (http://baheyeldin.com/khalid, and
http://2bits.com) to get rid of DHTML and Javascript, and make the
side bar a block.

If you use this module, find it useful, and want to send the author
a thank you note, then use the Feedback/Contact page at the URL above.

The author can also be contacted for paid customizations of this
and other modules.
