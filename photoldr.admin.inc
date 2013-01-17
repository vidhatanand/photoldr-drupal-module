<?php
/**
 * @file
 * Admin page callback for the PhotoLDR module.
 */

/**
 * Builds and returns the PhotoLDR admin page settings form.
 */
function photoldr_admin_settings() {
  $feed_back = "";

  // Add a list of links at the top of the admin page showing all menu links for
  // this module.
  $links = photoldr_menu();
  $nodeorder = array();
  $helptxt = t('<p><strong>PhotoLDR must have node types allowed.  Then users permissions must be set to allow users to create content, or the post will fail. </strong></p>');

  foreach ($links as $link => $v) {
    $helptxt .= '<p>' . l($v['title'] . ' - ' . $v['description'], $link) . '</p>';
  }
  $form['help'] = array(
    '#type' => 'markup',
    '#preface' => '<div>',
    '#markup' => $helptxt,
    '#suffix' => '</div>',
  );

  // Site settings for PhotoLDR.
  $form['photoldr_FQDN'] = array(
    '#type' => 'textfield',
    '#title' => 'FQDN',
    '#size' => 30,
    '#default_value' => variable_get('photoldr_FQDN', check_plain($_SERVER['HTTP_HOST'])),
    '#description' => t('Fully Qualified Domain Name of this web site.  Ex. www.Example.com'),
  );
  $form['photoldr_site_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Site Name'),
    '#size' => 30,
    '#default_value' => variable_get('photoldr_site_name', variable_get('site_name')),
    '#description' => t('Site name.  Remember this will be displayed on iPhone and iPad, so best to keep this short. (v2)'),
  );
  $form['photoldr_exp_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Expiration Date'),
    '#size' => 30,
    '#default_value' => variable_get('photoldr_exp_date', '+3 months'),
    '#description' => t('Enter a relative string like +3 months, +90 days, +1 year, or a static date as YYYY-MM-DD. </br> Expiration Date for data that is cached in the iOS app.  An expired domains data is removed from the app.  Useful if you are migrating to a new site, or shutting down a site, or have data that is time sensative. The app should attemt to refresh the data every day, but if the data is not refreshed for (+3 months, +90 days, +1 year) then the data is cleared from the app.'),
  );
  $form['photoldr_allow_unpublished'] = array(
    '#type' => 'select',
    '#title' => t('Unpublished Content'),
    '#options' => array('no' => 'No', 'yes' => 'Yes'),
    '#default_value' => variable_get('photoldr_allow_unpublished'),
    '#description' => t('Allow content editors with permission to work with unpublished content. (v2)'),
  );

  $nodenames = node_type_get_names();
  $defaultnodeslist = array('article', 'book', 'page', 'product_display');
  $form['photoldr_types'] = array(
    '#type' => 'select',
    '#multiple' => 'TRUE',
    '#title' => t('Node Types'),
    '#options' => $nodenames,
    '#size' => min(2, count($nodenames)),
    '#default_value' => variable_get('photoldr_types', $defaultnodeslist),
    '#description' => t('Select the types of nodes used by PhotoLDR. CTRL-click to select multiple.'),
  );

  // Allow the Site admin to pick image sizes to show in the iOS app.
  foreach (image_styles() as $style) {
    $styles[$style['name']] = $style['name'];
  }
  $form['photoldr_image_field_style'] = array(
    '#type' => 'select',
    '#title' => t('Image style'),
    '#options' => $styles,
    '#default_value' => variable_get('photoldr_image_field_style', array('medium' => 'medium')),
    '#description' => t('Select the style of images displayed by PhotoLDR.'),
  );
  $form['photoldr_image_icon_field_style'] = array(
    '#type' => 'select',
    '#title' => t('Icon style'),
    '#options' => $styles,
    '#default_value' => variable_get('photoldr_image_icon_field_style', array('thumbnail' => 'thumbnail')),
    '#description' => t('Select the image style of icon displayed by PhotoLDR.') . ' ' . l(t('Image Style Config'), 'admin/config/media/image-styles'),
  );

  // This is the default data/node type to create when a user adds data.
  // This is only useful if there is only one data type.
  // This may be removed... unless a value is found.
  reset($nodenames);
  $form['photoldr_default_post_type'] = array(
    '#type' => 'select',
    '#title' => t('Type of item to post as default'),
    '#options' => $nodenames,
    '#default_value' => variable_get('photoldr_default_post_type', 'page'),
    '#description' => t('Select the types of nodes to use PhotoLDR.  Revisit this page after setting this for more settings.'),
    '#suffix' => '<hr/>',
  );
  $form['photoldr_markup_message1'] = array(
    '#type' => 'markup',
    '#markup' => '<h3>' . t('Save Settings after changing any above options to refresh the available options bellow.') . '</h3>',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save and Refresh'),
    '#suffix' => '<hr/>',
  );

  // view_types is for a future feature in the iOS app which
  // allows a site admin to select how the data should be shown in the iOS app.
  $view_types = array('table' => 'Table', 'map' => 'Map -coming soon');
  $enabled_nodes = variable_get('photoldr_types', $defaultnodeslist);
  $weight_used = array();
  $count = 0;

  // This is a rough attempt at setting the order of the node types to display
  // in the XML. There is a better way I just need to find out how the Drupal
  // ordered weights is implemented.
  foreach ($enabled_nodes as $k => $nodename) {
    $weight = variable_get('photoldr_node_weight_' . $nodename, '');
    if ($weight == '') {
      $count++;
      while (in_array($count, $weight_used)) {
        $count++;
      }
      $weight = $count;
    }
    $weight_used[] = $weight;

    $form['photoldr_node_weight_' . $nodename] = array(
      '#type' => 'select',
      '#title' => t('@nodename - Weight.', array('@nodename' => $nodename)),
      '#options' => drupal_map_assoc(range(1, 50)),
      '#default_value' => $weight,
      '#description' => t('This will set the order that node types are presented to the iOS app.'),
    );

  }

  $form['photoldr_markup_table1'] = array(
    '#type' => 'markup',
    '#markup' => '<hr/>',
  );

  // This section presents node type specific settings for each node
  // in a three column table layaout.  This could be improved also.
  // The settings are very useful for a site admin to customize the delivery
  // to the iOS app.
  foreach ($enabled_nodes as $k => $nodename) {

    $form['photoldr_node_view_' . $nodename] = array(
      '#prefix' => '<table><tr><td>',
      '#suffix' => '</td>',
      '#type' => 'select',
      '#title' => t('@nodename - View Type', array('@nodename' => $nodename)),
      '#options' => $view_types,
      '#default_value' => variable_get('photoldr_node_view_' . $nodename, 'table'),
      '#description' => t('This will set the view type in the iOS app. CURRENTLY Disabled.  Only Tableview is used no matter what is set.'),
    );
    $form['photoldr_item_max_' . $nodename] = array(
      '#prefix' => '<td>',
      '#type' => 'select',
      '#title' => t('@nodename - Max items', array('@nodename' => $nodename)),
      '#options' => drupal_map_assoc(range(1, 100)),
      '#default_value' => variable_get('photoldr_item_max_' . $nodename, 10),
      '#description' => t('This will set the maximum number of items presented to the user.'),
    );
    $form['photoldr_item_age_max_' . $nodename] = array(
      '#suffix' => '</td>',
      '#type' => 'select',
      '#title' => t('@nodename - Max age (weeks)', array('@nodename' => $nodename)),
      '#options' => drupal_map_assoc(range(0, 52)),
      '#default_value' => variable_get('photoldr_item_age_max_' . $nodename, 0),
      '#description' => t('This will set the maximum allowable age in weeks of the displayed items.  0 is no max.'),
    );
    $form['photoldr_item_order_' . $nodename] = array(
      '#prefix' => '<td>',
      '#type' => 'select',
      '#title' => t('@nodename - Sort Order', array('@nodename' => $nodename)),
      '#options' => array(
        'DESC' => t('Descending Z-A or Newest First'),
        'ASC' => t('Ascending A-Z or Oldest First'),
      ),
      '#default_value' => variable_get('photoldr_item_order_' . $nodename, 'DESC'),
      '#description' => t('This will set the order which items are returned to the App.'),
    );
    $form['photoldr_item_orderby_' . $nodename] = array(
      '#type' => 'select',
      '#title' => t('@nodename - Sort By', array('@nodename' => $nodename)),
      '#options' => array(
        'created' => t('Created'),
        'changed' => t('Changed'),
        'title' => t('Title'),
        'nid' => t('Node ID'),
      ),
      '#default_value' => variable_get('photoldr_item_orderby_' . $nodename, 'created'),
      '#description' => t('This will set the field used to sort.'),
      '#suffix' => '</td></tr></table>',
    );

  }

  $form['photoldr_markup_table2'] = array(
    '#type' => 'markup',
    '#markup' => '',
  );

  // SHOW FIELD SETTINGS for PHOTOLDR APP.
  $feed_back .= '<br /> Items in <em style="color:red">Red</em>. are unsupported, but try the suggested input style. <br />nodetype:Label:field_name:type:options<br /><br />';

  // These modules/widget types are supported.
  $supported_fields = array(
    'image',
    'textfield',
    'text',
    'textarea',
    'checkbox',
    'date',
    'cck_phone',
    'addressfield',
  );
  // This is the added support for the addressfield type.
  // Currently only US formatted addresses are supported.
  $addressfields = array(
    'street1',
    'street2',
    'city',
    'state',
    'zip',
    'country',
  );

  // Build nodetype:Label:field_name:type:options
  $instances = field_info_instances('node');
  foreach ($instances as $ntype => $fields) {
    $type = node_type_get_type($ntype);
    if (isset($type->has_title)) {
      $feed_back .= $ntype . ':Title:title:textfield:<em style="color:blue">#required</em>';
      $feed_back .= '<br />' . "\n";
    }

    foreach ($fields as $instance) {
      $module = $instance['widget']['module'];
      $field_name = $instance['field_name'];
      $label = $instance['label'];

      if (!in_array($module, $supported_fields)) {

        if ($instance['widget']['type'] == 'options_select') {
          $feed_back .= $field_name . ', ' . $module . ', ' . $instance['widget']['type'] . ',  is not <em style="color:red"><strong>supported</strong></em>. ';
          $feed_back .= '<br />' . "\n";
          continue;
        }

        if ($instance['widget']['type'] == 'options_onoff') {
          $feed_back .= $ntype . ':';
          $feed_back .= $label . ':';
          $feed_back .= $field_name . ':';
          $feed_back .= 'checkbox:';
          $feed_back .= '<em>#true</em>';
          $feed_back .= '<br />' . "\n";
          continue;
        }

        $feed_back .= $ntype . ':';
        $feed_back .= $label . ':';
        $feed_back .= $field_name . ':';
        $feed_back .= 'textfield';
        $feed_back .= '          <strong>   ';
        $feed_back .= $module . ' ' . $instance['widget']['type'] . ' is not <em style="color:red">supported</em>.</strong> ';
        $feed_back .= '<br />' . "\n";
        continue;
      }

      $fieldinfo = field_info_field($instance['field_name']);

      if ($instance['widget']['module'] == 'addressfield') {

        foreach ($addressfields as $fieldname) {
          $feed_back .= $ntype . ':';
          $feed_back .= drupal_ucfirst($fieldname) . ':';
          $feed_back .= $fieldname . ':';
          $feed_back .= 'textfield';
          $feed_back .= '<br />' . "\n";
        }
        continue;
      }

      $feed_back .= $ntype . ':';
      $feed_back .= $label . ':';
      $feed_back .= $field_name . ':';
      switch ($module) {
        case 'date':
          $feed_back .= 'date:';
          break;

        case 'cck_phone':
          $feed_back .= 'textfield:';
          break;

        case 'text':
          if (isset($instance['widget']['settings']['rows'])) {
            $feed_back .= 'textarea:';
          }
          else {
            $feed_back .= 'textfield:';
          }
          break;

        default:
          $feed_back .= $module . ':';
          break;

      }

      if ($instance['required']) {
        $feed_back .= '<em style="color:blue">#required</em>' . '<br />' . "\n";
      }
      else {
        $feed_back .= '<br />' . "\n";
      }

      // End foreach ($fields as $instance).
    }

    $feed_back .= '<br />' . "\n";
    // END foreach ($instances as $instance).
  }

  // This is just for site developers to see how the node data is presented.
  $form['photoldr_admin'] = array(
    '#title' => 'Node Type Samples',
    '#type' => 'fieldset',
    '#weight' => '20',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['photoldr_admin']['samples'] = array(
    '#type' => 'markup',
    '#prefix' => '<strong>Sample format of data:</strong> <br/>',
    '#markup' => $feed_back,
    '#suffix' => '</hr>',
  );

  // Image Field Name selection.  This is depreciated and may be pruned soon.
  $fields = field_info_fields();
  $imagefields = array();
  foreach ($fields as $k => $v) {
    $module = $v['module'];
    if (in_array($module, array('image'))) {
      $imagefields[$k] = $k;
    }
  }
  reset($imagefields);
  $imgoptions = (count($imagefields) > 0) ? $imagefields : array('none found');
  $form['photoldr_admin']['photoldr_image_field'] = array(
    '#type' => 'markup',
    '#prefix' => '<strong>' . t('Image Fields in this site.') . '</strong><pre><br/>',
    '#markup' => print_r($imgoptions, TRUE),
    '#suffix' => '</pre>',
  );

  return system_settings_form($form);
}
