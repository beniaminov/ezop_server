-----------------------
GENERAL DESCRIPTION
-----------------------
This module allows you to set access permissions for various categories based
on user role.  

This works as a hook into node_access and follows the same general structure
as the node_access module.  It's got permissions for view, update and delete
for each category on the system.  


-----------------------
API functions
-----------------------

There are two ways you can check whether or not a user has
access for a given taxonomy.

First, you can call node_access() on a given node
and if taxonomy_access module is enabled it will take category permissions into
account when determining whether or not the user has access to the given node.

The second way is to call the function taxonomy_access() which takes two
parameters, an operation and a taxonomy, with a third optional paramater of a
userid to use instead of the current user. It returns whether or not the user
is authorized, almost identical to node_access().


-----------------------
DATABASE TABLES
-----------------------
Module creates two tables in database: 'term_access' and
'term_access_defaults'


-----------------------
HELP PAGES
-----------------------
For more information about how to control access permissions with the Taxonomy
Access Control module, see the module's help page (admin/help/taxonomy_access).


-----------------------
TROUBLESHOOTING:
-----------------------

When users can view/edit pages, they do not have permission:

1. Check if user role have 'administer nodes' permission.

2. Check if you have ANY 'node access' module installed on your system. 
    IMPORTANT: When using more 'node access' type module, 
      Permissions are ALWAYS OR-ed together.
      If one module grants permissions for a given user (role) for a node ,
      then user CAN view/edit/delete even if other module DENIES it.

3 Do a General Database Housekeeping
  (Tables: 'node_access','term_access' and 'term_access_defaults'):

  - First 'DISABLE', then 'RE-ENABLE' the 'Taxonomy Access Module' on page:
    "Administer >> Site building >> Modules".
    
  This will force the complete rebuild of the 'node_access' table.
  
4. For debugging, please install deve_node_access module (Devel module)
   This can show you some information about node_access values in 
   the database when viewing a node page.

5. Force rebuilding of the permissions cache (table 'node_access'):
    "Rebuild permissions" button on page: 
    "Administer >> Content Management >> Post settings".

    If the site is experiencing problems with permissions to content, you may
    have to rebuild the permissions cache. Possible causes for permission
    problems are disabling modules or configuration changes to permissions.
    Rebuilding will remove all privileges to posts, and replace them with
    permissions based on the current modules and settings.

-----------------------
UNINSTALL
-----------------------

1. After disabling module, you can make complete uninstall by choosing Taxonomy
   Access on page: "Administer >> Site building >> Modules >> Uninstall".

   This will remove all your settings of Taxonomy Access: variables and tables
   ('term_access' and 'term_access_defaults')

2. After uninstall, if the site is experiencing problems with permissions to
   content, you can rebuild the permission cache.
   See "Troubleshooting Section 5".

-----------------------
CONTACT
-----------------------

Please, send bug reports, feature requests, or other comments about Taxonomy
Access Control module: http://drupal.org/project/issues/taxonomy_access

You can also contact me personally:
Keve  ( http://drupal.org/user/13163/contact )
