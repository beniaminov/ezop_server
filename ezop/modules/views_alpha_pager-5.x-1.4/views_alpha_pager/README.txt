/* $Id: README.txt,v 1.3.2.1 2008/03/16 21:16:37 douggreen Exp $ */

views_alpha_pager is a simple module that implements an alphabet pager (A B C)
instead of a numeric pager (1 2 3) for views.

Installation
------------

Copy views_alpha_pager.module to your module directory and then enable on the
admin modules page.  Then, edit your view, and select the Pager option "Alpha".
Add a view "Sort" that returns the sorted field in an alphabetic sort (i.e.,
don't sort by id or date, but rather by title or author).
