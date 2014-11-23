// $Id: README.txt,v 1.3 2006/03/16 08:03:51 jaza Exp $

Readme
------

The activeselect module defines the activeselect form element. An activeselect element is the same as a regular select element, except that when the user selects a new option (or set of options), a different select element (the target element) gets its list updated. This is done using AJAX, and it is designed to degrade gracefully if the required JavaScript support is not present. The target element can be either a regular select box, or another activeselect box (which in turn can trigger another target box, which can trigger yet another, resulting in a hierarchical cascade of activeselect elements).

Notes: 

- Activeselect now supports multiple targets, each of which can be updated simultaneously with a different set of options.
- This module is of no use by itself. You only need to install this module if another module (e.g. the category module) instructs you to do so.
- The JavaScript library for this module is based heavily on the Drupal core AutoComplete JavaScript library (autocomplete.js). Much credit and thanks goes to the authors of AutoComplete.


Author
------

Jeremy Epstein <jazepstein@greenash.net.au>
