-- SUMMARY --

The photoldr module allows two way communication with an iOS app.

The iOS app reads XML from the PhotoLDR Module at /?q=photoldrstructure.xml
The iOS app can manipulate the data or create new data and submit
back to /?q=photoldrpost.php

The photoldr module pareses post data and updates or creates nodes.



For a full description of the module, visit the project page:
  http://drupal.org/sandbox/willisiw/1778338

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/node/add/project-issue/1778338


The XML Output is described in SAMPLE.txt and sample.xml

-- REQUIREMENTS --


-- INSTALLATION --

* Install module using the zip file.  For further info visit 
  http://drupal.org/documentation/install/modules-themes/modules-7

-- CONFIGURATION --

* After enabling the module, navigate to /?q=admin/config/photoldr

* Set the FQDN to a usable URL.  Case can be customized. 

* Set the expiration date to some date in the future. Relative string 
  is the best method. +1 month, +3 months, +1 year, +90 days

* Select the Node Types to be visible in the iOS app. CTRL-click to 
  select multiple.

* Select the Image Style to deliver to the iOS app as a Full Size image.
  Larger images may cause slower responce.

* Select the Icon style.  This should be thumbnail or some other small
  image style.  Preferrable 100x100 pixels or smaller.

* SAVE - Click SAVE on the bottom of the config page.  This will cause 
  the admin page to refresh presenting data specific to each Node type
  selected for display.

* Set the node type weights.  The weigts must be different, or a type 
  may not display.

* Specifiy settings per node type: Max Items, Max Age per item, Sort 
  Order, and Sort By.

* SAVE - Click SAVE on the bottom of the config page.



-- TROUBLESHOOTING --


-- FAQ --


-- CONTACT --

Current maintainers:
* Ian Willis (willisiw) - willisiw@754180.no-reply.drupal.org
