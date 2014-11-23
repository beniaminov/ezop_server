Taxonomy Defaults allows you to assign default terms from any vocabulary to any
node-type. 

INSTALLATION
------------

Copy all files to modules/taxonomy_defaults/, visit Admin >> Site building >>
Modules (admin/build/modules) and enable Taxonomy Defaults. The provided
install file should correctly set the weight of the module. If not execute the
following query manually:

UPDATE system SET weight = -1 WHERE name = 'taxonomy_defaults';


CONFIGURATION
-------------

Configure Taxonomy Defaults via Administer >> Content management, tab Defaults
(admin/content/taxonomy/taxonomy_defaults).

Each vocabulary is shown in a table for every content type. If you enable the
checkbox for a vocabulary, Taxonomy Defaults will add terms from the vocabulary
to the content type's submissions. If a vocabulary is activated for a content
type (shown by 'active' on the Taxonomy Defaults page), the terms will simply
be pre-selected on the add/node page. If not, then the terms will be added to
the node without any user interaction.

