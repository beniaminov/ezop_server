********************************************************************
                     D R U P A L    M O D U L E
********************************************************************
Name: News page module
Author: Robert Castelo
Drupal: 5.x
********************************************************************
DESCRIPTION:

Enables the creation of pages which show syndicated news
items from one aggregator category. The news items are filtered by
keywords (chosen when creating the page), so that only news items which
match the keyword(s) filter are shown.


Features include:

* Filter news items by keyword(s)
* AND/OR/NOT searches ('+', '-')
* 'Blog this' link for each news item
* Create RSS feed of news page





********************************************************************

PREREQUISITES:

Must have taxonomy module already enabled.



********************************************************************
USAGE:

1) Set up an aggregator category with feeds from sites you want to
    syndicate.

2) Create a news page
     - create content -> news page
     - select the aggregator category you want to filter. 
     - make sure "Input Format" is set to "Full HTML", otherwise the
        markup tags will be stripped out.
     - set the 'Include' keywords

Keyword Examples:

To display items which contain any one of the Japanese related words:

  ipod, nano, shuffle


To exclude news items based on keywords:

  ipod, nano, -shuffle


To include items which must contain a certain word:

  ipod, nano, shuffle, +apple




********************************************************************
INSTALLATION:

Note: It is assumed that you have Drupal up and running.  Be sure to
check the Drupal web site if you need assistance.

1. Place the entire news_page directory into your Drupal 
    modules/directory.

2. Enable the news_page module by navigating to:

     administer > modules
     
  Click the 'Save configuration' button at the bottom to commit your
  changes.
  
3. Set the roles that can create, edit and view news pages on the
    'access control' page

    www.your.site/admin/access
    
********************************************************************
AUTHOR CONTACT

- Report Bugs/Request Features:
   http://drupal.org/project/news_page
   
- Comission New Features:
   http://drupal.org/user/3555/contact
   
- Say Thank You:
   http://www.amazon.com/gp/registry/O6JKRQEQ774F

********************************************************************
CREDITS

Yannick Forest (efolia)
- Most of Drupal 5 update

manuj_78
- encouragement, testing and patience



