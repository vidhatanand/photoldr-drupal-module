<?php
/**
 * @file
 * Node Structure presented to PhotoLDR iOS App.
 *
 * /?q=photoldrstructure.xml
 * 
 * See SAMPLE.txt and sample.xml for the document structure.
 *
 * This presents this Druapl site as XML.
 */
  
  
  
/**
 * Presents XML page after checing user authentication via POST or GET variables
 * username and password.
 */
function photoldr_node_structure_page_auth() {
  $uid = _photoldr_html_user_auth();
  photoldr_node_structure_page($uid); 
}




/**
 * Presents XML page with the current Drupal authenicated user.
 * 
 */
function photoldr_node_structure_page($uidvar = NULL) {
  global $user;
  global $base_url;
  $uid = (isset($uidvar)) ? $uidvar : $user->uid; 
  $account = user_load($uid);

  $build = array();
  $output = '';

  $exp_date = variable_get('photoldr_exp_date', date("Y-m-d", strtotime('+3 year')));
  
  $fields = field_info_fields();
  $imagefields = array();
  $imagefieldmax = array();
  
  foreach ($fields as $k => $v) {
    $module = $v['module'];
    if (in_array($module, array('image'))) {
                $imagefieldinfo = field_info_field($k);
                switch ($imagefieldinfo['cardinality']) {
                  case FIELD_CARDINALITY_UNLIMITED:
                    $max = '10';
                    break;
                  default:
                    $max = $imagefieldinfo['cardinality'];
                    break;
                }
      $imagefields[$k] = $k;
      $imagefieldmax[$k] = $max;
    }
  }
  reset($imagefields);
  $imgoptions = (count($imagefields) > 0) ? $imagefields : array('none found');;




    /**
     * Accumulate the data to be converted to XML inside the $pub array
     * the base if the document <domains><domain> is the equivilent of the 
     * root of the $pub array.  So, $pub['FQDN'] will becomer the object at
     * <domains><domain><FQDN>...</FQDN>
     */
    $pub['FQDN'] = variable_get('photoldr_FQDN',  check_plain($_SERVER['HTTP_HOST']));
    $pub['site_name'] = variable_get('photoldr_site_name',  variable_get('site_name'));
    $pub['published'] = "1";
    $pub['cms'] = "drupal";
    $pub['standalone'] = "1";
    $pub['date'] = date( "Y-m-d H:i") ;
    $pub['exp_date'] = date("Y-m-d", strtotime($exp_date));
    $pub['exp_url'] = variable_get('photoldr_FQDN',  check_plain($_SERVER['HTTP_HOST']));
    $pub['post_url'] = "http://" . $pub['FQDN'] . "/?q=photoldrpost.php";
    /**
       * <app_options> exposes site specific options for user settings
       * in the iOS app
       */
      $pub['app_options'][] = "app:Username:username:textfield:";
      $pub['app_options'][] = "app:Password:password:password:";
      $pub['app_options'][] = "app:Publish:status:checkbox:";


  /**
   * <form_items> tag exposes field settings to PHOTOLDR APP
   * <node_types> tag will eventually allow configurable displays in the iOS app like
   *   table view, map view, or book view.
   */
  $form_items = '<form_items>' . "\n";
  $node_display_settings = '<node_types>' . "\n";

  $supported_fields = array('image', 'textfield', 'text', 'textarea', 'checkbox', 'date', 'cck_phone', 'addressfield');
  $addressfields = array('street1', 'street2', 'city', 'state', 'zip', 'country');
  $field_types = field_info_field_types();

    // Build nodetype:Label:field_name:type:options
    $instances = field_info_instances('node');
    foreach ($instances as $ntype => $fields) {
      $type = node_type_get_type($ntype);
      
      if ($uid == FALSE) {
        $usercreate = FALSE;
      }
      $access = 'create ' . $ntype . ' content';
      if (user_access($access, $account)) {
        $usercreate = TRUE;
      }
      else {
        $usercreate = FALSE;
      }

      $node_view = variable_get('photoldr_node_view_' . $ntype, 'table');
      $node_max = variable_get('photoldr_item_max_' . $ntype, 10);
      $node_age_max = variable_get('photoldr_item_age_max_' . $ntype, 0);
      $node_order = variable_get('photoldr_item_order_' . $ntype, 0);
      
      $node_display_settings .= '<option>' . $ntype . ':' . $node_view . '</option>' . "\n";

   /**
    * Add postURL option ONLY for user's with permission to create/add this type of node.
    * nodetype:Hidden:posturl:http://hostname/?q=photoldrpost.php
    */
    if ($usercreate) {
      $form_items .= '<option>' . $ntype . ':Hidden:posturl:' . $pub['post_url'] . '</option>';
      $form_items .= "\n";
    }

      /**
       * Basically all Drupal nodes have a title and body. Verify first then add option.
       */
      if (isset($type->has_title)) {
            $form_items .= '<option>' . $ntype . ':Title:title:textfield:#required</option>';
            $form_items .= "\n";
      }
      
      
       /**
         * Check the widget and setup the option for the field.
         * Also address unsupported fields.
         */
      foreach ($fields as $instance) {
        $module = $instance['widget']['module'];
        $label = $instance['label'];
        $field_name = $instance['field_name'];
        $fieldinfo = field_info_field($instance['field_name']);

        $this_node_type = $ntype;
        $this_field_label = $label;
        $this_field_name = $field_name;
        $this_field_type = '';
        $this_field_required = '';
        
        
        // Handle other or unsupported fields
        if (!in_array($module, $supported_fields)) {
          
          if ($instance['widget']['type'] == 'options_select') {
            // Option Select Fields
            $this_field_type = 'options_select';
            $this_field_required = 'UNSUPPORTED';
            
          }
          elseif ($instance['widget']['type'] == 'options_onoff') {
            // Checkbox
            $this_field_type = 'checkbox';
            $this_field_required = '#true';
          }
          else {
            // All other unsupported
            $this_field_type = 'textfield';
            $this_field_required = 'UNSUPPORTED ' . $instance['widget']['module'];
          }
          
            $form_items .= '<option>';
            $form_items .= $this_node_type . ':';
            $form_items .= $this_field_label . ':';
            $form_items .= $this_field_name . ':';
            $form_items .= $this_field_type . ':';
            $form_items .= $this_field_required ;
            $form_items .= '</option>' . "\n";
          continue;
        }
  

         /**
          * Special support for the addressfield module.
          */
        if ($instance['widget']['module'] == 'addressfield') {
          
          foreach ($addressfields as $fieldname) {
            $this_field_label = drupal_ucfirst($fieldname);
            $this_field_name = $fieldname;
            $this_field_type = 'textfield';
            
            
            $form_items .= '<option>';
            $form_items .= $this_node_type . ':';
            $form_items .= $this_field_label . ':';
            $form_items .= $this_field_name . ':';
            $form_items .= $this_field_type . ':';
            $form_items .= $this_field_required ;
            $form_items .= '</option>' . "\n";
          }
        continue;
        }
        
          /**
          * Special support for the date module.
          */
        if ($instance['widget']['module'] == 'date') {
          
            $this_field_type = 'date';
            
            $form_items .= '<option>';
            $form_items .= $this_node_type . ':';
            $form_items .= $this_field_label . ':';
            $form_items .= $this_field_name . ':';
            $form_items .= $this_field_type . ':';
            $form_items .= $this_field_required ;
            $form_items .= '</option>' . "\n";
            
            $form_items .= '<option>';
            $form_items .= $this_node_type . ':';
            $form_items .= $this_field_label . ' End:';
            $form_items .= $this_field_name . '_end:';
            $form_items .= $this_field_type . ':';
            $form_items .= $this_field_required ;
            $form_items .= '</option>' . "\n";
          
        continue;
        }
  
  

    switch ($module) {
          case 'image':
                if ($imagefieldmax[$this_field_name] > 3) {
                  $this_field_type = 'image';
                }
                else {
                  $this_field_type = 'image';
                }
                $this_field_required = '#' . $imagefieldmax[$this_field_name];

                break;
          case 'cck_phone':
          case 'text':
              if (isset($instance['widget']['settings']['rows'])) {
                $this_field_type = 'textarea';
              }
              else {
                $this_field_type = 'textfield';
              }
              break;
          default:
                $this_field_type = $module;
                break;
    } // end of switch statement
  
          if ($instance['required']) {
            $this_field_required = '#required';
          }

          $form_items .= '<option>';
          $form_items .= $this_node_type . ':';
          $form_items .= $this_field_label . ':';
          $form_items .= $this_field_name . ':';
          $form_items .= $this_field_type . ':';
          $form_items .= $this_field_required ;
          $form_items .= '</option>' . "\n";
  
  
          if ($module == 'image') {
            $form_items .= '<option>' . $ntype . ':Replace Photos:image_overwrite:checkbox:</option>';
          }
        }  // end foreach field
        
        
        
        
    }  // END foreach ($instances as $instance)
    $form_items .= '</form_items>' . "\n";
    $node_display_settings .= '</node_types>' . "\n";
    
    
   /**
    * This is the actual conversion of the $pub array to the <domain> XML element.
    */
    $output .= '<domain>' . "\n";

    // Move code to function  <domain> site fields are in $pub
    $output .= _photoldr_domain_to_xml($pub);
    
    // These are the settings for the iOS app add/edit forms
    $output .= $form_items;

    $output .= $node_display_settings;

    // Add <nodes> tag Content.
    $output .= _photoldr_xml_node_content();
    
    if (variable_get('photoldr_allow_unpublished', 'no') != 'no') {
      // Add <nodes> tag Content for unpublished nodes.
      $output .= _photoldr_xml_unpublished_node_content($account);
    }
    // Close the <domain> tag in the document.
    $output .= '</domain>' . "\n";


  $xml_head = '<?xml version="1.0" encoding="utf-8"?>' . "\n";
  /* $xml_head .= '<?xml-stylesheet href="XMLViewer.css" type="text/css"?>'. "\n"; */
  $xml_head .= '<domains src="photoldr_node_structure_page">' . "\n";
  $xml_foot = '</domains>';

  drupal_add_http_header('Content-Type', 'application/xml');

  print($xml_head);
  print($output);
  print($xml_foot);

}




/**
 * Get published nodes and convert them to XML.
 *
 * This function queries the database for ndoes and returns XML.
 *
 *
 * @return
 *   String xml document body.  This string should be enclosed in a document tag 
 *   and have an XML header added to the display.  In this document context, this
 *   portion of XML will reside withing the <domain></domain> key.
 *   
 */
function _photoldr_xml_node_content() {
  global $user;
  global $language;
  $langcode = isset($langcode) ? $langcode : $language->language;
  $restrictedfields = array('pass', 'data', 'rdf_mapping', 'roles' );
  $pub = array();
  $data = array();
  
  $defaultnodeslist = array('article', 'book', 'page', 'product_display');
  $enabled_nodes = variable_get('photoldr_types', $defaultnodeslist);
  foreach ($enabled_nodes as $k => $nodename)  {
    $node_order[$nodename] = variable_get('photoldr_node_weight_' . $nodename);
  }
  asort($node_order);
  

  
  if (isset($_POST['changed'])) {
    $data['changed'] = check_plain($_POST['changed']);
  }
  elseif (isset($_GET['changed'])) {
    $data['changed'] = check_plain($_GET['changed']);
  }

  $uidvar = _photoldr_html_user_auth();
  $uid = ((isset($uidvar)) && ($uidvar)) ? $uidvar : $user->uid; 
  $account = user_load($uid);
  // END user authentication section

  $pub = _photoldr_account_to_xml($account, $pub);

  // Get list of nodes to publish and act on the list
  foreach ($node_order as $nodename => $k) {
      unset($query);
      unset($result);
      $node_type = node_type_get_type($nodename);
      $node_max = variable_get('photoldr_item_max_' . $nodename, 10);
      $node_age_max = variable_get('photoldr_item_age_max_' . $nodename, 0);
      $node_order = variable_get('photoldr_item_order_' . $nodename, 'DESC');
      $node_order_by = variable_get('photoldr_item_orderby_' . $nodename, 'created');

      if ($node_order == 'DESC') {
        $sort = 'DESC';
      }
  else {
        $sort = 'ASC';
      }
      

    // Query nodes owned by this user
    $query = db_select('node', 'n');
    $query
      ->fields('n')
      ->condition('n.status', 1)
      ->condition('n.type', $nodename);
    $query
      ->orderBy('n.' . $node_order_by, $sort);

    if (isset($data['changed'])) {
      $oldest_date = date(strtotime($data['changed']));
      $query->condition('n.changed', $oldest_date, "<");
    }
    elseif (isset($node_age_max) && ($node_age_max != 0)) {
      $oldest_date = date(strtotime('-' . $node_age_max . ' weeks'));
      $query->condition('n.created', $oldest_date, '>');
    }

    if (isset($node_max) && ($node_max != 0)) {
      $query->range('0', $node_max);
    }

    $result = $query->execute();
    
    $pub = _photoldr_prep_node_for_xml($result, $pub, $account);

  } // end foreach node type
  
    // make XML from $pub
  $xml_body = _photoldr_nodes_to_xml($pub, $restrictedfields);
  
    // debug post data.  st uid to -10 to disable
  if ($uid == 1) {
    $handle = fopen("pubdata.txt", "a");
    fwrite($handle, 'Pub: ' . print_r($pub, true) . '  ');
    fclose($handle);
  }
  return $xml_body;

}




/**
 * Get unpublished nodes and convert them to XML.
 *
 * This function queries the database for ndoes and returns XML.
 *
 * @return
 *   String xml document body.  This string should be enclosed in a document tag 
 *   and have an XML header added to the display.  In this document context, this
 *   portion of XML will reside withing the <domain></domain> key.  For no access
 *   or no results, returns empty string.
 *   
 */
function _photoldr_xml_unpublished_node_content($account) {
  /**  This can help show permissions
  foreach (module_list(FALSE, FALSE, TRUE) as $module) {
    print_r($module);
    
    // Drupal 7
    if ($permissions = module_invoke($module, 'permission')) {
      print_r($permissions);
    }
  }
  */
  $unpublished = FALSE;
  $userunpublished = FALSE;
  $access = 'administer nodes';
  $access1 = 'view own unpublished content';
  if (user_access($access, $account)) {
    $unpublished = TRUE;
  }
  elseif (user_access($access1, $account)) {
    $userunpublished = TRUE;
  }
  else {
    // return empty string for no results.
    return '';
  }
  
  
  global $user;
  global $language;
  $langcode = isset($langcode) ? $langcode : $language->language;
  $restrictedfields = array('pass', 'data', 'rdf_mapping', 'roles' );
  $pub = array();
  $data = array();
  
  $defaultnodeslist = array('article', 'book', 'page', 'product_display');
  $enabled_nodes = variable_get('photoldr_types', $defaultnodeslist);
  foreach ($enabled_nodes as $k => $nodename)  {
    $node_order[$nodename] = variable_get('photoldr_node_weight_' . $nodename);
  }
  asort($node_order);

  drupal_set_message('UID: ' . $user->uid);
  $uidvar = _photoldr_html_user_auth();
  $uid = ((isset($uidvar)) && ($uidvar)) ? $uidvar : $user->uid; 
  $account = user_load($uid);
  // END user authentication section

  // Get list of nodes to publish and act on the list
  foreach ($node_order as $nodename => $k) {
      unset($query);
      unset($result);
      $node_type = node_type_get_type($nodename);
      $node_max = variable_get('photoldr_item_max_' . $nodename, 10);
      $node_age_max = variable_get('photoldr_item_age_max_' . $nodename, 0);
      $node_order = variable_get('photoldr_item_order_' . $nodename, 'DESC');
      $node_order_by = variable_get('photoldr_item_orderby_' . $nodename, 'created');

      if ($node_order == 'DESC') {
        $sort = 'DESC';
      }
      else {
        $sort = 'ASC';
      }
      

    // Query nodes owned by this user
    $query = db_select('node', 'n');
    $query
      ->fields('n')
      ->condition('n.type', $nodename);
    $query
      ->orderBy('n.' . $node_order_by, $sort);
    
    //Decide what unpublished nodes to display.
    if ($unpublished) {
      $query->condition('n.status', 0);
    } 
    elseif ($userunpublished) {
      $query
          ->condition('n.status', 0)
          ->condition('n.uid', $uid, '=');
    }

    if (isset($node_max) && ($node_max != 0)) {
      $query->range('0', $node_max);
    }

    $result = $query->execute();
    
    $pub = _photoldr_prep_node_for_xml($result, $pub, $account);

  } // end foreach node type
  
  
    // make XML from $pub
  $xml_body = _photoldr_nodes_to_xml($pub, $restrictedfields);
  

  return $xml_body;

}
