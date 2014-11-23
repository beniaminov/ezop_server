CCK Field Permissions Module

This module allows an admin to restrict CCK fields to user roles. 
You can control who has access to create content in the fields you identify, edit own content, edit content, and view content. 

USAGE
Go to admin/settings/cck_field_perms to enable which fields you'd like control
Once you have enabled a content type, select which fields you'd like to restrict. 
Then, go to admin/user/access to allow user roles acces to that field. 

Please note: you must grant view access for a field once you enable this field to be permissions contoled. 
If you do not do this, no users will be able to view the field.

Known Issues:
page content types-  (and probably story) don't support this well out of the box. I haven't experimented with adding additional fields to page content types, but it seems that this module will not work with the default setup
matrix module- doesn't hide the table, but does hide the data